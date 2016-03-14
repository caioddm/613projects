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
	regions[0].base_address = _base_address;
	regions[0].size = _size;
	regions[0].available = TRUE;
	/*Initialize the allocated regions array with a single element representing the entire vm pool as available*/
}

unsigned long VMPool::allocate(unsigned long _size){
	unsigned long base_addr = 0;
	unsigned long remainingSize = 0;
	unsigned long sizeToAllocate = _size % PageTable::PAGE_SIZE > 0 ? ((_size / PageTable::PAGE_SIZE) + 1) * PageTable::PAGE_SIZE : _size; //computes the size of the region, rounding to the next multiple of page size
	int i;
	for (i = 0; i < region_index; ++i) //iterate over the allocated regions to look if any of the previously allocate ones is available now
	{	
		if(regions[i].available == TRUE && regions[i].size >= sizeToAllocate){ //found a region that was released
			base_addr = regions[i].base_address;
			if(i < _MAX_NO_REGIONS - 1){ //there is still room to split this region in two if size needed is less than the current size of the region
				remainingSize = regions[i].size - sizeToAllocate; //compute the remaining size when allocating just the size needed
				regions[i].size = sizeToAllocate;
				regions[i].available = FALSE;
				if(remainingSize > 0){ //creates a new available region with the remaining size computed
					for(int j = region_index; j > i; j--){ //shift all allocated regions one space to the right to make room for the newly created region
						regions[j].base_address = regions[j-1].base_address;
						regions[j].size = regions[j-1].size;
						regions[j].available = regions[j-1].available;
					}
					regions[i+1].base_address = regions[region_index].base_address + regions[region_index].size;
					regions[i+1].size = remainingSize;
					regions[i+1].available = TRUE;
					/* creates the new available region and set it to available*/
					region_index++;
				}
				break;
			}
			else{ //allocated regions array is already full and thus no split is possible, simply mark the region as unavailable
				regions[i].available = FALSE;
				break;
			}
		}
	}
	
	
	
	if(i == region_index){ //reached the last region allocated without finding any available spot
		if(regions[region_index].available == TRUE && regions[i].size >= sizeToAllocate){ //check if the last allocated region is available and if it has enough size to accommodate the request 
			base_addr = regions[region_index].base_address;
			if(region_index < _MAX_NO_REGIONS - 1){ //there is still room to split this region in two if size needed is less than the current size of the region
				regions[(region_index+1)].available = TRUE;
				regions[(region_index+1)].base_address = regions[region_index].base_address + sizeToAllocate;
				remainingSize = regions[region_index].size - sizeToAllocate;
				regions[(region_index+1)].size = remainingSize;
				/*creates a new region with the remaining size and set it to available*/
				
				regions[region_index].size = sizeToAllocate;
				regions[region_index].available = FALSE;
				regions[region_index].base_address = base_addr;
				region_index++;
				/*mark the region with the proper size and set available to false*/
			}
			else{ //allocated regions array is already full and thus no split is possible, simply mark the region as unavailable
				regions[region_index].available = FALSE;
			}
		}
		else{ //no region available to allocate
			Console::puts("Error when allocating the region!\n");
			return 0;
		}
	}
	
	return base_addr;
}

void VMPool::release(unsigned long _start_address){
	int index = -1;
	for (int i = 0; i < region_index; ++i) //search for the region that is going to be released and store the index of it
	{
		if(regions[i].base_address == _start_address){
			index = i;
			break;
		}
	}
	if(index != -1) {
		for (int i = _start_address; i < regions[i].base_address + regions[i].size; i += PageTable::PAGE_SIZE) //free all the pages within that region
		{
			pageTable->free_page(_start_address / PageTable::PAGE_SIZE);
		}
		
		regions[index].available = TRUE; //mark the region as available
		pageTable->load(); //flush the TLB
	}
}

BOOLEAN VMPool::is_legitimate(unsigned long _address){
	for (int i = 0; i < _MAX_NO_REGIONS; ++i) //iterate over the allocated regions to look if the address is valid
	{
		if(_address >= regions[i].base_address && _address < (regions[i].base_address + regions[i].size) && regions[i].available == FALSE){ //_address is within the region and region is not available
			return TRUE;
		}
	}

	return FALSE;
}