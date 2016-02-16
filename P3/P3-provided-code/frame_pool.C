#include "frame_pool.H"

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

#define KERNEL_BASE_ADDR (2 MB)
#define PROCESS_BASE_ADDR (4 MB)
/* definition of base addresses for the kernel and process pools*/

#define FRAME_SIZE (4 KB)
/* definition of the frame size */

unsigned char* FramePool::kernel_frame_bitmap;
unsigned long FramePool::kernel_info_frame_no;
unsigned char* FramePool::proc_frame_bitmap;
unsigned long FramePool::proc_info_frame_no;
/* Static variables definition */

FramePool::FramePool(unsigned long _base_frame_no, unsigned long _nframes,unsigned long _info_frame_no)
{
	if(_base_frame_no == KERNEL_POOL_START_FRAME) //start kernel frame pool
	{
		kernel_info_frame_no = _info_frame_no;
		/* Allocates the kernel bitmap to the appropriated address */
		if(kernel_info_frame_no != 0)
			kernel_frame_bitmap = (unsigned char *)KERNEL_BASE_ADDR + ((kernel_info_frame_no - KERNEL_POOL_START_FRAME) * FRAME_SIZE);
		else
		{
			kernel_info_frame_no = KERNEL_POOL_START_FRAME;
			kernel_frame_bitmap = (unsigned char *)KERNEL_BASE_ADDR;
		}		
		
		for(int i = 0; i < _nframes; i++)
			kernel_frame_bitmap[i/8] = kernel_frame_bitmap[i/8] & ~(1 << (i % 8)); //set all the frames on the kernel pool to unused
		
		if(_info_frame_no == 0)
			kernel_frame_bitmap[0] = kernel_frame_bitmap[0] | 1<<0; //mark the first frame of the kernel pool as used
		else
			kernel_frame_bitmap[(kernel_info_frame_no - KERNEL_POOL_START_FRAME)/8] = 
				kernel_frame_bitmap[(kernel_info_frame_no - KERNEL_POOL_START_FRAME)/8] | 1<<((kernel_info_frame_no - KERNEL_POOL_START_FRAME) % 8); //mark the frame of the kernel pool where the bitmap is stored as used
		
		/*set the data structures for the current instance*/
		frame_bitmap = kernel_frame_bitmap;
		info_frame_no = kernel_info_frame_no;
		n_frames = _nframes;
		base_frame_no = _base_frame_no;
	}
	else if(_base_frame_no == PROCESS_POOL_START_FRAME) //start process frame pool
	{
		if(_info_frame_no == 0)
		{
			for(int i = 0; i < KERNEL_POOL_SIZE; i++) //search for an available frame to hold the process bitmap
			{
				if(kernel_frame_bitmap[i/8] & (1 << (i % 8)) == FALSE)
				{
					kernel_frame_bitmap[i/8] = kernel_frame_bitmap[i/8] | (1 << (i % 8)); // mark the frame as used
					_info_frame_no = i + KERNEL_POOL_START_FRAME;
					break;
				}
			}
		}

		proc_info_frame_no = _info_frame_no;
		/* Allocates the process bitmap to the appropriated address */
		proc_frame_bitmap = (unsigned char *)KERNEL_BASE_ADDR + ((proc_info_frame_no - KERNEL_POOL_START_FRAME) * FRAME_SIZE);
		for(int i = 0; i < _nframes; i++)
			proc_frame_bitmap[i/8] = proc_frame_bitmap[i/8] & ~(1 << (i % 8)); //set all the frames on the process pool to unused

		/*set the data structures for the current instance*/
		frame_bitmap = proc_frame_bitmap;
		info_frame_no = proc_info_frame_no;
		n_frames = _nframes;
		base_frame_no = _base_frame_no;
	}
}

unsigned long FramePool::get_frame()
{
	for (int i = 0; i < n_frames ; i++)
      if (!(frame_bitmap[i/8] & (1 << (i % 8)))) //search for an available frame whithin the instance frame pool
      {
      	frame_bitmap[i/8] = frame_bitmap[i/8] | (1 << (i % 8)); // mark the frame as used
		return i + base_frame_no; // return the frame number
      }        
	
	return 0; //no available frame was found
}

void FramePool::mark_inaccessible(unsigned long _base_frame_no, unsigned long _nframes)
{
	unsigned long start_frame = _base_frame_no;
	unsigned long end_frame = _base_frame_no + _nframes - 1; 
	if(start_frame >= base_frame_no && end_frame < (base_frame_no + n_frames))
	{
		_base_frame_no = _base_frame_no - base_frame_no; //compute the frame number according to the bitmap index
		for (int i = _base_frame_no; i < (_base_frame_no + _nframes) ; i++)
      		frame_bitmap[i/8] = frame_bitmap[i/8] | (1 << (i % 8)); // mark all the frames in the inaccessible area as used
	}
	else{
		Console::puts("Invalid range to mark as inaccessible! \n");
	}
}

void FramePool::release_frame(unsigned long _frame_no)
{	
	if(_frame_no < MEM_HOLE_START_FRAME || _frame_no >= (MEM_HOLE_START_FRAME + MEM_HOLE_SIZE)) //make sure that an inaccessible frame is not being released
	{
		if(_frame_no >= PROCESS_POOL_START_FRAME && _frame_no < (PROCESS_POOL_START_FRAME + PROCESS_POOL_SIZE)) // frame to be released belongs to the process pool
		{
			if(_frame_no == proc_info_frame_no){
				Console::puts("Invalid frame to be released! \n");
			}
			else{
				_frame_no = _frame_no - PROCESS_POOL_START_FRAME; //compute the frame number according to the bitmap index
				proc_frame_bitmap[_frame_no/8] = proc_frame_bitmap[_frame_no/8] & ~(1 << (_frame_no % 8)); //mark the frame as unused	
			}			
		}
		else if(_frame_no >= KERNEL_POOL_START_FRAME && _frame_no < (KERNEL_POOL_START_FRAME + KERNEL_POOL_SIZE)) // frame to be released belongs to the kernel pool
		{
			if(_frame_no == kernel_info_frame_no){
				Console::puts("Invalid frame to be released! \n");
			}
			else{
				_frame_no = _frame_no - KERNEL_POOL_START_FRAME; //compute the frame number according to the bitmap index
				kernel_frame_bitmap[_frame_no/8] = kernel_frame_bitmap[_frame_no/8] & ~(1 << (_frame_no % 8)); //mark the frame as unused
			}			
		}
		else{
			Console::puts("Invalid frame to be released! \n");
		}
	}
	else{
		Console::puts("Invalid frame to be released! \n");
	}
}
