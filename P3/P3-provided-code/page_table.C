#include "page_table.H"

#define MB * (0x1 << 20)
#define KB * (0x1 << 10)
#define KERNEL_POOL_START_FRAME ((2 MB) / FRAME_SIZE)
#define KERNEL_POOL_SIZE ((2 MB) / FRAME_SIZE)
#define PROCESS_POOL_START_FRAME ((4 MB) / FRAME_SIZE)
#define PROCESS_POOL_SIZE ((28 MB) / FRAME_SIZE)
/* definition of the kernel and process memory pools */

#define MEM_HOLE_START_FRAME ((15 MB) / FRAME_SIZE)
#define MEM_HOLE_SIZE ((1 MB) / FRAME_SIZE)
/* we have a 1 MB hole in physical memory starting at address 15 MB */

#define KERNEL_BASE_ADDR (2 MB)
#define PROCESS_BASE_ADDR (4 MB)
/* definition of base addresses for the kernel and process pools*/

#define FRAME_SIZE (4 KB)
/* definition of the frame size */

#define PAGE_TABLE_SIZE (0x1 << 10)
/* definition of the number of entries on a page table */

PageTable* PageTable::current_page_table;
FramePool* PageTable::kernel_mem_pool;
FramePool* PageTable::process_mem_pool;
unsigned long PageTable::shared_size;
unsigned int PageTable::paging_enabled; 
/* Static variables definition */

void PageTable::init_paging(FramePool * _kernel_mem_pool, FramePool * _process_mem_pool, const unsigned long _shared_size){
	kernel_mem_pool = _kernel_mem_pool;
	process_mem_pool = _process_mem_pool;
	shared_size = _shared_size;
	paging_enabled = 0;
	/* set the global variables and mark the paging as disabled */
}

PageTable::PageTable(){
	page_directory = (unsigned long*) (kernel_mem_pool->get_frame() * FRAME_SIZE); //set the address of the page directory
	unsigned long* page_table = (unsigned long*) (kernel_mem_pool->get_frame() * FRAME_SIZE); //set the address of the first page table in the page directory

	unsigned long addr = 0;

	for (int i = 0; i < PAGE_TABLE_SIZE; ++i)
	{
		page_table[i] = addr | 3; //maps the address and set entry to be present, supervisor level, and read/write
		addr = addr + FRAME_SIZE; // set the address to be the start of the next frame
	}

	page_directory[0] = (unsigned long)page_table; // sets the page table just created to be the first entry on the page directory
	page_directory[0] |= 3; // set entry to be present, supervisor level, and read/write

	for (int i = 1; i < PAGE_TABLE_SIZE; ++i)
	{
		page_directory[i] = 0 | 2; //set all the other entries on the page directory to be not present, supervisor level, and read/write
	}

}

void PageTable::load(){
	current_page_table = this; //set the current page table object loaded
	write_cr3((unsigned long)(page_directory)); //load the page directory on the proper register	
}

void PageTable::enable_paging(){
	paging_enabled = 1; //mark paging as enabled
	write_cr0(read_cr0() | 0x80000000); // set the proper paging bit to 1
}

void PageTable::handle_fault(REGS * _r){
	unsigned long error_code = _r->err_code; 
	unsigned long fault_addr = read_cr2();
	/* read the error code and the fault address*/

	int directory_entry = fault_addr >> 22; //gets the entry on the page directory
	int table_entry = (fault_addr >> 12 & 0x03FF); //gets the entry on the corresponding page table
	unsigned long* _page_directory = current_page_table->page_directory; // gets the page directory of the current page table loaded in the CR3 register

	if(error_code & 1){ // Protection fault happened
		Console::puts("Protection fault!\n");
	}
	else{ //Page is not present
		if(_page_directory[directory_entry] & 1){ //page table that contains the address is present in the page directory
			unsigned long* page_table = (unsigned long*)(_page_directory[directory_entry] & 0xFFFFF000); //get the page table from the page directory, ensuring that it brings the starting address of the page table
			page_table[table_entry] = process_mem_pool->get_frame() * FRAME_SIZE; //allocate a frame from the process pool and map to the page table entry
			page_table[table_entry] |= 7; //set entry to be present, user level, and read/write
		}
		else{ //page table needs to be loaded in the page directory
			unsigned long* page_table = (unsigned long*) (kernel_mem_pool->get_frame() * FRAME_SIZE); //allocate a frame from the kernel pool to hold the created page table
			for (int i = 0; i < PAGE_TABLE_SIZE; ++i)
			{
				page_table[i] = 0; //set all the entries of the created page table to be empty
			}

			page_table[table_entry] = process_mem_pool->get_frame() * FRAME_SIZE; //allocate a frame from the process pool and map to the page table entry
			page_table[table_entry] |= 7; //set entry to be present, user level, and read/write

			_page_directory[directory_entry] = (unsigned long)page_table; //map the created page table to the proper page directory entry
			_page_directory[directory_entry] |= 3; // set entry to be present, supervisor level, and read/write	
		}	
	}	
	
}