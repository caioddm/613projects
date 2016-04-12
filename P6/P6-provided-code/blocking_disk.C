#include "blocking_disk.H"


BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size): SimpleDisk(_disk_id,_size){
  
}

void BlockingDisk::wait_until_ready() {
  while (!is_ready()) 
  { 
    SYSTEM_SCHEDULER->resume(Thread::CurrentThread()); //put the current thread at the end of the list
    SYSTEM_SCHEDULER->yield(); //pass the cpu to the next thread on teh queue
  }
}
/*
void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf){
  SimpleDisk::read(_block_no, _buf);
}

void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf){
  SimpleDisk::write(_block_no, _buf);
}*/