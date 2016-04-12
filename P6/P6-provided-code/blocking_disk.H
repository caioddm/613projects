#ifndef BLOCKING_DISK_H
#define BLOCKING_DISK_H

#include "simple_disk.H"
#include "scheduler.H"
#include "utils.H"

extern Scheduler* SYSTEM_SCHEDULER;

class BlockingDisk : public SimpleDisk{
    protected:
        //OVERRIDE
        void wait_until_ready();

    public:
        BlockingDisk(DISK_ID _disk_id, unsigned int _size);
        /* Creates a SimpleDisk device with the given size connected to the MASTER
           or SLAVE slot of the primary ATA controller. */

        /*DISK OPERATIONS */
        //void read(unsigned long _block_no, unsigned char * _buf);
        /* Reads 512 Bytes from the given block of the disk and copies them
            to the given buffer. No error check! */
        //void write(unsigned long _block_no, unsigned char * _buf);
        /* Writes 512 Bytes from the buffer to the given block on the disk. */

};

#endif