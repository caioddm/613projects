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

VMPool::VMPool(unsigned long _base_address, unsigned long _size, FramePool *_frame_pool, PageTable *_page_table){

}

unsigned long VMPool::allocate(unsigned long _size){
}

void VMPool::release(unsigned long _start_address){
}

BOOLEAN VMPool::is_legitimate(unsigned long _address){

}