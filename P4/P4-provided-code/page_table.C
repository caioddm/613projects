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
const unsigned int PageTable::PAGE_SIZE;
const unsigned int PageTable::ENTRIES_PER_PAGE;
/* Static variables definition */

void PageTable::init_paging(FramePool * _kernel_mem_pool, FramePool * _process_mem_pool, const unsigned long _shared_size){
	kernel_mem_pool = _kernel_mem_pool;
	process_mem_pool = _process_mem_pool;
	shared_size = _shared_size;
	paging_enabled = 0;
	/* set the global variables and mark the paging as disabled */
}

PageTable::PageTable(){
	page_directory = (unsigned long*) (process_mem_pool->get_frame() * PAGE_SIZE); //set the address of the page directory
	unsigned long* page_table = (unsigned long*) (process_mem_pool->get_frame() * PAGE_SIZE); //set the address of the first page table in the page directory

	unsigned long addr = 0;

	for (int i = 0; i < ENTRIES_PER_PAGE; ++i)
	{
		page_table[i] = addr | 3; //maps the address and set entry to be present, supervisor level, and read/write
		addr = addr + PAGE_SIZE; // set the address to be the start of the next frame
	}

	page_directory[0] = (unsigned long)page_table; // sets the page table just created to be the first entry on the page directory
	page_directory[0] |= 3; // set entry to be present, supervisor level, and read/write

	for (int i = 1; i < ENTRIES_PER_PAGE - 1; ++i)
	{
		page_directory[i] = 0 | 2; //set all the other entries on the page directory to be not present, supervisor level, and read/write
	}

	page_directory[ENTRIES_PER_PAGE - 1] = (unsigned long)page_directory; //Sets the last entry on the page directory to point to the page directory itself
	page_directory[ENTRIES_PER_PAGE - 1] |= 3;

	vm_pools = (VMPool**) (process_mem_pool->get_frame() * PAGE_SIZE); //allocate the memory for the vm pools list
	pool_index = 0; //indicates that the vm pool is empty
}

void PageTable::load(){
	current_page_table = this; //set the current page table object to be the current instance
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
	unsigned long* _page_directory = (unsigned long*) 0xFFFFF000; // gets the page directory of the current page table based on the virtual address

	if(error_code & 1){ // Protection fault happened
		Console::puts("Protection fault!\n");
	}
	else{ //Page is not present
		if(_page_directory[directory_entry] & 1){ //page table that contains the address is present in the page directory
			unsigned long* page_table = (unsigned long *) (0xFFC00000 + (directory_entry * PAGE_SIZE)); //get the page table from the virtual address
			unsigned long addr = process_mem_pool->get_frame() * PAGE_SIZE; //allocate a frame from the process pool
			//check if the frame allocation was successfull
			if(addr == 0){ 
				Console::puts("No frame available on the memory!\n");
			}
			else{
				page_table[table_entry] = addr; // map that frame to the page table entry
				page_table[table_entry] |= 7; //set entry to be present, user level, and read/write	
			}			
		}
		else{ //page table needs to be loaded in the page directory
			unsigned long pt_addr = (process_mem_pool->get_frame() * PAGE_SIZE); //allocate a frame from the process pool to hold the created page table
			//check if the frame allocation was successfull
			if(pt_addr == 0){
				Console::puts("No frame available on the memory!\n");	
			}
			else{				
				unsigned long* page_table = (unsigned long *) (0xFFC00000 + (directory_entry * PAGE_SIZE)); //get the page table from the virtual address
				for (int i = 0; i < ENTRIES_PER_PAGE; ++i)
				{
					page_table[i] = 0; //set all the entries of the created page table to be empty
				}
				unsigned long addr = process_mem_pool->get_frame() * PAGE_SIZE; //allocate a frame from the process pool
				//check if the frame allocation was successfull
				if(addr == 0){
					Console::puts("No frame available on the memory!\n");			
				}
				else{
					page_table[table_entry] = addr; //map that frame to the page table entry
					page_table[table_entry] |= 7; //set entry to be present, user level, and read/write

					_page_directory[directory_entry] = (unsigned long) pt_addr; //map the created page table to the proper page directory entry
					_page_directory[directory_entry] |= 3; // set entry to be present, supervisor level, and read/write	
				}				
			}
			
		}	
	}	
	
}

void PageTable::free_page(unsigned long _page_no){
	unsigned long virtual_address = _page_no * PAGE_SIZE; //gets the virtual address of the specified page number
	int directory_entry = virtual_address >> 22; //gets the page table entry on the page directory
	int table_entry = (virtual_address >> 12 & 0x03FF); //gets the entry on the corresponding page table
	unsigned long* page_table = (unsigned long *) (0xFFC00000 + (directory_entry * PAGE_SIZE)); // access the page table that contains the specified page number

	if(page_table[table_entry] & 1){ //page table entry is present
		process_mem_pool->release_frame(page_table[table_entry]); //release the frame corresponding to the page
		page_table[table_entry] = 0; //clear the page table entry to indicate it is not valid anymore
	}
	else{
		Console::puts("Page is already free");
	}
}

void PageTable::register_vmpool(VMPool *_pool){
	vm_pools[pool_index] = _pool; //register the virtual memory pool and increment the pool_index
	pool_index++;
}