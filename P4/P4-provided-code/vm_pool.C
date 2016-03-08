#include "vm_pool.H"

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


const unsigned int VMPool::_MAX_NO_REGIONS;
/* Static variables definition */


VMPool::VMPool(unsigned long _base_address, unsigned long _size, FramePool *_frame_pool, PageTable *_page_table){
	/*store all the necessary properties*/
	base_address = _base_address;
	size = _size;
	framePool = _frame_pool;
	pageTable = _page_table;
	region_index = 0;
	_page_table->register_vmpool(this); //register the VM Pool on the page table
}

unsigned long VMPool::allocate(unsigned long _size){
	if(_size == 0 || region_index >= _MAX_NO_REGIONS){ //check if the size is 0 or if the regions list is full
		Console::puts("Error when allocating the region!\n");
		return 0;
	}else{
		unsigned long base_addr = base_address;
		if(region_index > 0){
			base_addr = regions[region_index - 1]->base_address + regions[region_index - 1]->size; //computes the base address of the region to be allocated			
		}
		
		if(_size % PageTable::PAGE_SIZE > 0){
			_size = ((_size / PageTable::PAGE_SIZE) + 1) * PageTable::PAGE_SIZE; //computes the size of the region, rounding to the next multiple of page size
		}			
		/* set the allocated region properties */
		regions[region_index]->base_address = base_addr;
		regions[region_index]->size = _size;
		region_index++;
		return base_addr;
	}
}

void VMPool::release(unsigned long _start_address){
	int index = 0;
	for (int i = 0; i < region_index; ++i) //search for the region that is going to be released and store it on the variable region
	{
		if(regions[i]->base_address == _start_address){
			index = i;
			break;
		}
	}

	for (int i = _start_address; i < regions[i]->base_address + regions[i]->size; i += PageTable::PAGE_SIZE) //free all the pages within that region
	{
		pageTable->free_page(_start_address / PageTable::PAGE_SIZE);
	}

	for (int i = index + 1; i < region_index; ++i) //shift all the regions after the one released one position back on the array
	{
		regions[i-1]->base_address = regions[i]->base_address;
		regions[i-1]->size = regions[i]->size;
	}
	region_index--;
	pageTable->load();
}

BOOLEAN VMPool::is_legitimate(unsigned long _address){
	for (int i = 0; i < region_index; ++i) //iterate over the allocated regions to look if the address is valid
	{
		if(_address >= regions[i]->base_address && _address < (regions[i]->base_address + regions[i]->size)){ //_address is within the region
			return TRUE;
		}
	}

	return FALSE;
}