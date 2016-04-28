#ifndef BLOCKING_DISK_H
#define BLOCKING_DISK_H

#include "simple_disk.H"
#include "scheduler.H"
#include "utils.H"

extern Scheduler* SYSTEM_SCHEDULER;

//methods to implement a lock to be used on the thread-safe disk and file system bonus option 3
static void Lock(BOOLEAN& lock)
{
    while(lock){
        Console::puts("Waiting for another disk/file system operation... yield CPU to another thread!\n");
        SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
        SYSTEM_SCHEDULER->yield(); //pass the cpu to the next thread on the queue
    }

    lock = TRUE;
}

static void Unlock(BOOLEAN& lock)
{
    lock = FALSE;
}

class BlockingDisk : public SimpleDisk{
        BOOLEAN busy;
    protected:
        //OVERRIDE
        void wait_until_ready();

    public:
        BlockingDisk(DISK_ID _disk_id, unsigned int _size);
        /* Creates a SimpleDisk device with the given size connected to the MASTER
           or SLAVE slot of the primary ATA controller. */

        /*DISK OPERATIONS */
        void read(unsigned long _block_no, unsigned char * _buf);
        /* Reads 512 Bytes from the given block of the disk and copies them
            to the given buffer. No error check! */
        void write(unsigned long _block_no, unsigned char * _buf);
        /* Writes 512 Bytes from the buffer to the given block on the disk. */

};

#endif