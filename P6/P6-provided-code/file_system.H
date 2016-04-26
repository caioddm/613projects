#include "blocking_disk.H"
#include "utils.H"

#define BLOCK_SIZE 512
#define SYSTEM_DISK_SIZE 10485760
#define SYSTEM_BLOCKS (SYSTEM_DISK_SIZE/BLOCK_SIZE)
#define HEADER_SIZE 8

#define FREE    0x0000
#define USED    0xFFFF
#define BLOCK_DATA_SIZE 126 //126*4 bytes

class FileSystem;
extern FileSystem* FILE_SYSTEM;
/* forward declaration of file system class and global variable*/

static unsigned char disk_buffer[BLOCK_SIZE];//used for transfering data with the hard disk
struct DiskBlock{
	unsigned int id; //file id
	unsigned int availability;//used or free
	unsigned int data[126];//block file data
	/*each file is contained in one block, and each block has a 8 byte header, made of the file id and availability*/
};

static DiskBlock* disk_block=(DiskBlock*)disk_buffer;//cast disk buffer to the DiskBlock struct to access the variables

class File {
	friend class FileSystem;
	/* -- your file data structures here ... */
	//FileSystem   * file_system;

     unsigned int   file_id;
     unsigned int 	block_num; //number of the block containing the file
     unsigned int   current_position; //current position in the block
	
	public:
	File();
	File(unsigned int fileID);
	/* Constructor for the file handle. Set the ’current position’
	* to be at the beginning of the file.
	*/
	unsigned int Read(unsigned int _n, char * _buf);
	/* Read _n characters from the file starting at the current
	* location and copy them in _buf. Updates ‘current position’
	* Return the number of characters read.
	*/
	void Write(unsigned int _n, char * _buf);
	/* Write _n characters to the file starting at the current
	* location; if we run past the end of file, we increase the
	* size of the file as needed. Updates ‘current position’.
	*/
	void Reset();
	/* Set the ’current position’ at the beginning of the file.
	*/
	void Rewrite();
	/* Erase the content of the file. Return any freed blocks.
	* Note: This function does not delete the file!
	* It just erases its content.
	*/
	BOOLEAN EoF();
	/* Is the current location for the file at the end of the file? */
};

class FileSystem {
	friend class File;
	/* your file system data structures here ... */
	 SimpleDisk * disk; //disk accessed by the file system
     static unsigned int size; //size of the file system
     unsigned int blocks_available; //block to look for free blocks
	 File* files; //keeps track of the files in the file system
	 unsigned int files_size; //number of files in the file system
	 
	public:
	FileSystem();
	/* Just initializes local data structures.
	* Does not connect to disk yet.
	*/
	BOOLEAN Mount(SimpleDisk * _disk);
	/* Associates the file system with a disk.
	* Limit to at most one file system per disk.
	* Returns TRUE if operation successful (i.e. there is indeed
	* a file system on the disk.
	*/
	static BOOLEAN Format(SimpleDisk * _disk, unsigned int _size);
	/* Wipes any file system from the given disk and installs an
	* empty file system of given size.
	*/
	BOOLEAN LookupFile(int _file_id, File * _file);
	/* Find file with given id in file system.
	* If found, initialize the file object and return TRUE.
	* Otherwise, return FALSE.
	*/
	BOOLEAN CreateFile(int _file_id);
	/* Create file with given id in the file system.
	* If file exists already, abort and return FALSE.
	* Otherwise, return TRUE.
	*/
	BOOLEAN DeleteFile(int _file_id);
	/* Delete file with given id in the file system and free any
	* disk block occupied by the file.
	*/
	int AllocateBlock();
	/* Allocates a block from free list, for use in file */
	void FreeBlock(unsigned int block_index);
	/* Deallocates a block from free list, for use in file */
	void AddFile(File* file);
	/* Adds the file to the files array */
};