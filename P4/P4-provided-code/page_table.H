/*
    File: page_table.H

    Author: R. Bettati, Nikhil Gupta
            Department of Computer Science
            Texas A&M University
    Date  : 10/03/25

    Description: Basic Paging.

*/

#ifndef _page_table_H_                   // include file only once
#define _page_table_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define KB * (0x1 << 10)
#define FRAME_SIZE (4 KB)

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "exceptions.H"
#include "frame_pool.H"
#include "paging_low.H"
#include "vm_pool.H"

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* Forward declaration of class VMPool */
class VMPool;

/*--------------------------------------------------------------------------*/
/* P A G E - T A B L E  */
/*--------------------------------------------------------------------------*/

class PageTable {

private:

  /* THESE MEMBERS ARE COMMON TO ENTIRE PAGING SUBSYSTEM */
  static PageTable     * current_page_table; /* pointer to currently loaded page table object */
  static unsigned int    paging_enabled;     /* is paging turned on (i.e. are addresses logical)? */
  static FramePool     * kernel_mem_pool;    /* Frame pool for the kernel memory */
  static FramePool     * process_mem_pool;   /* Frame pool for the process memory */
  static unsigned long   shared_size;        /* size of shared address space */

  /* DATA FOR CURRENT PAGE TABLE */
  unsigned long        * page_directory;     /* where is page directory located? */
  
  static const unsigned int _MAX_NO_POOLS = FRAME_SIZE/sizeof(VMPool*); //define how many entries of VM Pools can be stored on one frame
  VMPool** vm_pools; /*keeps track of all the registered vm pools*/
  unsigned int pool_index; /*index of the next vm pool to be registered*/

public:
  static const unsigned int PAGE_SIZE        = Machine::PAGE_SIZE; /* in bytes */
  static const unsigned int ENTRIES_PER_PAGE = Machine::PT_ENTRIES_PER_PAGE; /* in entries, duh! */  

  static void init_paging(FramePool * _kernel_mem_pool,
                          FramePool * _process_mem_pool,
                          const unsigned long _shared_size);
  /* Set the global parameters for the paging subsystem. */

  PageTable();
  /* Initializes a page table with a given location for the directory and the
     page table proper.
     NOTE: The PageTable object still needs to be stored somewhere! Probably it is best
           to have it on the stack, as there is no memory manager yet...
     NOTE2: It may also be simpler to create the first page table *before* paging
           has been enabled.
  */

  void load();
  /* Makes the given page table the current table. This must be done once during
     system startup and whenever the address space is switched (e.g. during
     process switching). */

  static void enable_paging();
  /* Enable paging on the CPU. Typically, a CPU start with paging disabled, and
     memory is accessed by addressing physical memory directly. After paging is
     enabled, memory is addressed logically. */

  static void handle_fault(REGS * _r);
  /* The page fault handler. */

  void free_page(unsigned long _page_no);
  /* Release the frame associated with the page _page_no */

  void register_vmpool(VMPool * _pool);
  /* The page table needs to know about where it gets its pages from.
     For this, we have VMPools register with the page table. */

};

#endif

