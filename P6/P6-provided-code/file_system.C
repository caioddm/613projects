#include "file_system.H"

File::File(){ //default constructor, initialize a blank file object
}

File::File(unsigned int fileID){ //initialize a file identified by its ID
	file_id=fileID;
	if (FILE_SYSTEM->LookupFile(file_id, this))
		Console::puts("Found File\n");
	else if (!FILE_SYSTEM->CreateFile(file_id))
		Console::puts("error creating file\n");
}

unsigned int File::Read(unsigned int _n, char * _buf){
	unsigned int count=_n;//initialize count
	while (count>0){
		if (EoF() && count>0)
			Console::puts("error reading the file\n");
			
		FILE_SYSTEM->disk->read(block_nums[current_block],disk_buffer);//read block from file
		for (current_position; current_position < BLOCK_SIZE-HEADER_SIZE; current_position++){//cur position ranges from 0-511 in increments of 8
			if (count==1)//do not read more than were supposed to
				break;

			memcpy(_buf,disk_buffer+HEADER_SIZE+current_position,1);//copy from file  buffer to user buffer
			++_buf;//increment buffer pointer
			count-=1; //decrement remaining chars to be read

			if (current_position==(BLOCK_SIZE - HEADER_SIZE)){//reached end of block, move to the next one
				current_position=0;
				++current_block;
			}
		}
	}
	return _n - count;//returns the total amount read
}

void File::Write(unsigned int _n, char * _buf){
	unsigned int count=_n;//initialize count
	while (count > 0){
		if (EoF()){//reached end of file, get new block
			unsigned int new_block_num=FILE_SYSTEM->AllocateBlock(); //allocate a new block on the file system
			unsigned int* new_blocks= (unsigned int*)new unsigned int[file_size+1];
			
			if (block_nums!=NULL){
				for (unsigned int i=0;i<file_size;++i)//copy old list of block pointers
					new_blocks[i]=block_nums[i];
					
				new_blocks[file_size]=new_block_num;//set new index to new block number
			}
			else
				new_blocks[0]=new_block_num;
				
			++file_size;//increment file size
			delete block_nums; //delete old array
			block_nums=new_blocks;//set pointer to new array
		}
			
		memcpy((void*)(disk_buffer+HEADER_SIZE),_buf,BLOCK_SIZE-HEADER_SIZE);//writes buffer content into disk_buffer
		FILE_SYSTEM->disk->write(block_nums[current_block],disk_buffer); //write disk buff into disk
		count-=(BLOCK_SIZE-HEADER_SIZE); //decrease the remaining chars to be written
	}
}

void File::Reset(){
	/* set current position and current block to 0 */
	current_position=0;
	current_block=0;
}

void File::Rewrite(){
	current_block=0; //set current block to start of file;
	while(current_block < file_size){ /* iterate over all the blocks of the file, deallocating them */
		FILE_SYSTEM->FreeBlock(block_nums[current_block]);
		++current_block;
	}
	/* reset the variables to the initial File state */
	file_size=0;
	block_nums=NULL;
	current_block=0;
	current_position=0;
}

BOOLEAN File::EoF(){
	if (block_nums==NULL)
		return TRUE;
		
	if (current_block == file_size-1 && current_position == BLOCK_SIZE-HEADER_SIZE-1 )
		return TRUE;
	else
		return FALSE;
}

FileSystem::FileSystem(){
	/* initialize all the variables to zero */
	blocks_available=0;
	files_size=0;
	files=NULL;
	memset(disk_buffer,0,BLOCK_SIZE);//clear buffer
}

BOOLEAN FileSystem::Mount(SimpleDisk * _disk){
	disk=_disk;
	disk->read(0,disk_buffer);
	files_size=disk_block->size;
	for (unsigned int i=0;i<files_size;++i){
		disk->read(0,disk_buffer);//refresh buffer back to root node of file system
		File* newFile= new File();//create a new file
		disk->read(disk_block->data[i],disk_buffer);//puts file metadata in disk buffer
		newFile->file_size=disk_block->size;
		newFile->file_id=disk_block->id;
		/*unsigned int k=0;
		for ( k=0;k<newFile->file_size;++k){
		   // newFile->GetBlock();//allocates a disk_block
		   // newFile->block_nums[j]=inode->blocks[j];//sets disk_block number
		}*/
		AddFile(newFile);
	}
}

BOOLEAN FileSystem::Format(SimpleDisk * _disk, unsigned int _size){
	//every file uses one disk_block as its inode, not the most efficient, but better to implement
    //can support files up to about 64 KB 127*512
    //first disk_block of filesystem is always reserved for an array of inodes with more if needed
    //meaning our system will theoretically support up to 2^32 files but realistically we will long run out of memory before then
    //first 4 bytes of the first disk_block is the number of files disk_block
    /*testing FILE_SYSTEM->disk->read(50,disk_buffer);
            putUIntData(disk_buffer,10*sizeof(unsigned int));
            Console::puts("\nRead initial\n");
    memset(disk_buffer,'b',15);
      FILE_SYSTEM->disk->write(50,disk_buffer);
            putUIntData(disk_buffer+HEADER_SIZE,10*sizeof(unsigned int));
            Console::puts("\nWrote\n");
    memset(disk_buffer,0x0,BLOCK_SIZE);
    FILE_SYSTEM->disk->read(50,disk_buffer);
            putUIntData(disk_buffer+HEADER_SIZE,10*sizeof(unsigned int));
            Console::puts("\nAFTER WRITE\n");
    assert(false); */
    memset(disk_buffer,0,BLOCK_SIZE);//set entire disk to 0, automatically free memory
    Console::puts("Formatting Disk, may take awhile\n");
    for (int i=0;i<SYSTEM_BLOCKS;++i)
        _disk->write(i,disk_buffer);
		
	_disk->read(0, disk_buffer);
    disk_block->availability=USED;//set disk_block to used
    disk_block->size=0;//write to  size disk_block this will cause first 4 bytes to be empty which is interpereted as 0 files on disk
    _disk->write(0,disk_buffer);//initializes master disk_block
    Console::puts("Format of empty file system complete\n");
}

BOOLEAN FileSystem::LookupFile(int _file_id, File * _file){	
	for (unsigned int i=0; i < files_size; i++){
		if (files[i].file_id==_file_id){ //found the file, set the file pointer and return true
			*_file=files[i];
			return TRUE;
		}
	}
	
	return FALSE; //did not find the file, return false
}

BOOLEAN FileSystem::CreateFile(int _file_id){
	File* newFile = new File();

	if (LookupFile(_file_id,newFile))
		return FALSE; //file already exists
		
	newFile->file_id=_file_id;
	newFile->file_size=0;
	newFile->block_nums=NULL;
	//newFile->Rewrite();//simply clears all data and sets fields to 0
	//do not need to handle allocating data, write function of file will take care of that
	//but we do need to create an inode
	newFile->info_block_num = AllocateBlock();//allocate a disk_block to hold the metadata
	disk->read(newFile->info_block_num,disk_buffer);//load disk_block into disk buffer
	disk_block->availability=USED;//set metadata values and write it back to disk
	disk_block->size=0;//size 0
	disk_block->id=_file_id;
	disk->write(newFile->info_block_num,disk_buffer);//write file inode to disk
	AddFile(newFile);//add file to the files array
	return TRUE;
}

BOOLEAN FileSystem::DeleteFile(int _file_id){
	File* file;
	if (!LookupFile(_file_id,file)) //file doesn't exist, return false
		return FALSE;
	
	File* new_file_array= (File*)new File[files_size-1];
	BOOLEAN found = FALSE;
	unsigned int j = 0;
	for (unsigned int i=0;i<files_size-1;++i){//copy elements to the new files array		
		if (files[i].file_id==_file_id){
			found==TRUE;
			files[i].Rewrite();//erases and unallocate memory
			FreeBlock(files[i].info_block_num);//deletes metadata of file
			j++; //skips copying the deleted file
		}		
		new_file_array[i]=files[j];
		j++;
	}
	files_size--;//decrement the number of files
	delete files; //delete old array
	files=new_file_array;//set pointer to new array
	if (files_size==0)
		files=NULL; //files array is empty
		
	return found;	
}

unsigned int FileSystem::AllocateBlock(){
	disk->read(blocks_available,disk_buffer);
	BOOLEAN fromStart=FALSE;
	while (disk_block->availability == USED){
		if (blocks_available>(SYSTEM_BLOCKS-1)){//try again starting from the beginning
			blocks_available=0;
			if (fromStart)
				return 0; //did not find any disk_block on the entire disk
			
			fromStart = TRUE;
		}
		++blocks_available;
		disk->read(blocks_available,disk_buffer);
	}
	disk->read(blocks_available,disk_buffer);
	disk_block->availability=USED;//sets disk_block header to used
	disk->write(blocks_available,disk_buffer);
	return blocks_available;
}

void FileSystem::FreeBlock(unsigned int block_index){
	disk->read(block_index,disk_buffer);
	disk_block->availability=FREE;//sets disk_block status to free
	disk->write(blocks_available,disk_buffer);
}

void FileSystem::AddFile(File* file){
	if (files=NULL)
		files=file;
	else
	{
		File* new_file_array = (File*)new File[files_size+1];
		unsigned int i = 0;
		for (i=0;i<files_size;++i)//copy files from the old array to the new one
			new_file_array[i] = files[i];
			
		new_file_array[files_size] = *file; //append the file parameter at the end of the array
		++files_size;//increment files size
		delete files; //delete old array
		files=new_file_array;//update files pointer
	}
}