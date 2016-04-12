#include "blocking_disk.H"


BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size): SimpleDisk(_disk_id,_size){
  
}

void BlockingDisk::wait_until_ready() {
  while (!is_ready()) 
  { 
    Console::puts("not ready...\n"); 
    //SYSTEM_SCHEDULER->resume(Thread::CurrentThread());//place current thread on ready queue
    //SYSTEM_SCHEDULER->yield();//yield processor while we wait for io to be ready
  }
  Console::puts("done!!!\n");
}

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf){
  SimpleDisk::read(_block_no, _buf);
}

void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf){
  SimpleDisk::write(_block_no, _buf);
}