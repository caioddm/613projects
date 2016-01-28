/*
    File: kernel.C

    Author: Original from R. Bettati, modified by  Dilma Da Silva
            Department of Computer Science and Engineering
            Texas A&M University
	 
    Original Date  : 12/09/03
    Modifications:   01/26/16

    This file has the main entry point to the operating system.

*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define MB * (0x1 << 20)
#define KB * (0x1 << 10)
#define KERNEL_POOL_START_FRAME ((2 MB) / (4 KB))
#define KERNEL_POOL_SIZE ((2 MB) / (4 KB))
#define PROCESS_POOL_START_FRAME ((4 MB) / (4 KB))
#define PROCESS_POOL_SIZE ((28 MB) / (4 KB))
/* definition of the kernel and process memory pools */

#define MEM_HOLE_START_FRAME ((15 MB) / (4 KB))
#define MEM_HOLE_SIZE ((1 MB) / (4 KB))
/* we have a 1 MB hole in physical memory starting at address 15 MB */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "debug.H"
#include "machine.H"     /* LOW-LEVEL STUFF   */
#include "console.H"

#include "frame_pool.H"

/*--------------------------------------------------------------------------*/
/* MAIN ENTRY INTO THE OS */
/*--------------------------------------------------------------------------*/

int main() {
    Console::init();

    /* For P3, we will need to initialize exception handling here. */

    /* -- INITIALIZE FRAME POOLS -- */

    FramePool kernel_mem_pool(KERNEL_POOL_START_FRAME,
                              KERNEL_POOL_SIZE,
                              0);
    unsigned long process_mem_pool_info_frame = kernel_mem_pool.get_frame();
    FramePool process_mem_pool(PROCESS_POOL_START_FRAME,
                               PROCESS_POOL_SIZE,
                               process_mem_pool_info_frame);
    process_mem_pool.mark_inaccessible(MEM_HOLE_START_FRAME, MEM_HOLE_SIZE);

    /* -- INITIALIZE MEMORY (PAGING) -- */
    /* Next project will do this */

    /* -- ENABLE INTERRUPTS -- */
    /* not yet ... but in P3 we will need to do that */

    /* -- MOST OF WHAT WE NEED IS SETUP. THE KERNEL CAN START. */

    Console::puts("Hello World, we are conquering P2!\n");

    /* HERE WRITE YOUR OWN CODE TO TEST THE FRAME POOL CLASS
     * IMPLEMENTATION WITH THE TWO OBJECTS WE ALLOCATED - ON THE
     * STACK, WE DO NOT HAVE "NEW" YET - ABOVE */



    /* -- NOW LOOP FOREVER */
    Console::puts("Testing is done. We will do nothing forever\n");
    for(;;);

    /* -- WE DO THE FOLLOWING TO KEEP THE COMPILER HAPPY. */
    return 1;
}
