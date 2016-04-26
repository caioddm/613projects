#include "file_system.H"

unsigned int FileSystem::size;

File::File(){ //default constructor, initialize a blank file object
	current_position = 0;
}

File::File(unsigned int fileID){ //initialize a file identified by its ID and add it to the file system files array
	file_id=fileID;
	if (FILE_SYSTEM->LookupFile(file_id, this))
		Console::puts("Found File\n");
	else if (!FILE_SYSTEM->CreateFile(file_id))
		Console::puts("error creating file\n");
}

unsigned int File::Read(unsigned int _n, char * _buf){
	unsigned int count=_n;//initialize count
	while (count>0){
		if (EoF() && count > 0)
			Console::puts("reached end of file before reading all chars\n");
			
		FILE_SYSTEM->disk->read(block_num,disk_buffer);//read block from file
		for (current_position; current_position < BLOCK_SIZE-HEADER_SIZE; current_position++){//read file data
			if (count == 0)//read all the requested bytes
				break;

			memcpy(_buf,disk_buffer + HEADER_SIZE + current_position,1);//copy onefrom file  buffer to user buffer
			++_buf;//increment buffer pointer
			count-= 1; //decrement remaining chars to be read
		}
	}
	return _n - count;//returns the total amount read
}

void File::Write(unsigned int _n, char * _buf){
	unsigned int count=_n;//initialize count
	while (count > 0){
		if (EoF() && count > 0){//reached end of file, get new block
			Console::puts("reached end of file before writing all chars\n");
		}
		
		FILE_SYSTEM->disk->read(block_num,disk_buffer);//read block from file
		for (current_position; current_position < BLOCK_SIZE-HEADER_SIZE; current_position++){//write file data one byte at a time into buffer
			if (count == 0)//written all the requested bytes
				break;

			memcpy((void*)(disk_buffer + HEADER_SIZE + current_position),_buf,1);//writes buffer byte into disk_buffer			
			++_buf;//increment buffer pointer
			count-= 1; //decrease the remaining chars to be written
		}
	}
	if(count == 0) //able to perform the write on disk
		FILE_SYSTEM->disk->write(block_num, disk_buffer); //write disk buff into disk
}

void File::Reset(){
	/* set current position to 0 */
	current_position = 0;
}

void File::Rewrite(){
	/* Since we are constraining the file to one block, we simply need to reset the current position */
	Reset();
}

BOOLEAN File::EoF(){		
	if (current_position == BLOCK_SIZE-HEADER_SIZE-1 )
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
	files_size=0;
	for (unsigned int i = 0;i < size; i++){ //iterate over disk blocks
		disk->read(i,disk_buffer);//read disk block
		if(disk_block->availability == USED){ //there is a file on this block
			File* newFile= new File();//create a new file
			newFile->file_id = disk_block->id;
			newFile->block_num = i;
			AddFile(newFile);
		}		
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
    size = _size/BLOCK_SIZE; //set the size of the formatted disk in blocks
    memset(disk_buffer,0,BLOCK_SIZE);
    Console::puts("Formatting Disk, may take awhile\n");
    if(size > SYSTEM_BLOCKS) //if desired size is greater than the physical space, limit it to physical disk space
    	size = SYSTEM_BLOCKS;

    for (int i=0;i<size;++i) //set formatted disk to 0, automatically free memory
        _disk->write(i,disk_buffer);
		
	/*_disk->read(0, disk_buffer);
    disk_block->availability=USED;//set disk_block to used
    disk_block->size=0;//write to  size disk_block this will cause first 4 bytes to be empty which is interpereted as 0 files on disk
    _disk->write(0,disk_buffer);//initializes master disk_block   */ 
    Console::puts("Format of empty file system complete\n");
}

BOOLEAN FileSystem::LookupFile(int _file_id, File * _file){	
	for (unsigned int i=0; i < files_size && i < size; i++){
		if (files[i].file_id==_file_id){ //found the file, set the file pointer and return true
			*_file=files[i];
			return TRUE;
		}
	}
	
	return FALSE; //did not find the file, return false
}

BOOLEAN FileSystem::CreateFile(int _file_id){
	File* newFile = NULL;
	if (LookupFile(_file_id,newFile))
		return FALSE; //file already exists

	int available_block = AllocateBlock(); //allocate a block for the new file
	if(available_block == -1)
		return FALSE; //file system is full
		

	newFile = new File();
	newFile->file_id=_file_id;
	newFile->Rewrite(); //ensures current position is at the beginning	
	newFile->block_num = available_block;//allocate a disk_block to hold the metadata
	disk->read(newFile->block_num,disk_buffer);//load disk_block into disk buffer
	disk_block->availability = USED;//set metadata values and write it back to disk
	disk_block->id = _file_id;
	disk->write(newFile->block_num,disk_buffer);//write file inode to disk
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
			FreeBlock(files[i].block_num);//mark block as available
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

int FileSystem::AllocateBlock(){
	disk->read(blocks_available,disk_buffer);
	BOOLEAN fromStart=FALSE;
	while (disk_block->availability == USED){
		if (blocks_available>(size-1)){//try again starting from the beginning
			blocks_available = 0;
			if (fromStart)
				return -1; //did not find any disk_block on the entire disk
			
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
	disk->write(block_index,disk_buffer);
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