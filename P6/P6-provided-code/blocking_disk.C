#include "blocking_disk.H"


BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size): SimpleDisk(_disk_id,_size){
  
}

void BlockingDisk::wait_until_ready() {
  while (!is_ready()) 
  { 
    //when waiting for the disk operation to be ready, we will emqueue this thread on the blocked queue and yield the CPU to the next available thread
    SYSTEM_SCHEDULER->resumeBlocked(Thread::CurrentThread());
    SYSTEM_SCHEDULER->yield(); //pass the cpu to the next thread on the queue
  }
}
/*
void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf){
  SimpleDisk::read(_block_no, _buf);
}

void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf){
  SimpleDisk::write(_block_no, _buf);
}*/