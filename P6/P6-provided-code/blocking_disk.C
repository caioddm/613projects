#include "blocking_disk.H"


BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size): SimpleDisk(_disk_id,_size){

}

void BlockingDisk::wait_until_ready() {
}

void BlockingDisk::read(unsigned long _block_no, char * _buf){
}

void BlockingDisk::write(unsigned long _block_no, char * _buf){
}