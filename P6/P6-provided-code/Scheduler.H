/* 
    Author: Joshua Capehart
			Based on code by:
			R. Bettati
            Department of Computer Science
            Texas A&M University
			
			A thread scheduler.

*/
#ifndef SCHEDULER_H
#define SCHEDULER_H

/*--------------------------------------------------------------------------*/
/* DEFINES 
/*--------------------------------------------------------------------------*/

#define SCHE_ROUND_ROBIN //enable round robin scheduling

/*--------------------------------------------------------------------------*/
/* INCLUDES 
/*--------------------------------------------------------------------------*/

#include "machine.H"
#include "thread.H"
#include "utils.H"
#include "console.H"

/*--------------------------------------------------------------------------*/
/* SCHEDULER
/*--------------------------------------------------------------------------*/

struct TaskNode{ //data structure to hold the queue of tasks ready
	Thread *thread;
	TaskNode *next;
	TaskNode(Thread* t) : thread(t){}
};

class Scheduler {

   /* The scheduler may need private members... */
	int queue_size; //store how many elements are on the queue;
   int blocked_queue_size; //store how many elements are on the blocked threads queue;
	TaskNode* queue; //store the queue
   TaskNode* blockedQueue; //store the queue of blocked threads
	void enqueue(Thread* _thread);
   void enqueueBlocked(Thread* _thread);
public:
   Scheduler();
   /* Setup the scheduler. This sets up the ready queue, for example.
      If the scheduler implements some sort of round-robin scheme, then the 
      end_of_quantum handler is installed here as well. */

   virtual void yield();
   /* Called by the currently running thread in order to give up the CPU. 
      The scheduler selects the next thread from the ready queue to load onto 
      the CPU, and calls the dispatcher function defined in 'threads.h' to
      do the context switch. */

   virtual void resumeBlocked(Thread * _thread); //called whenever we want to resume a thread waiting for an blocking disk operation
   virtual void resume(Thread * _thread);
   /* Add the given thread to the ready queue of the scheduler. This is called
      for threads that were waiting for an event to happen, or that have 
      to give up the CPU in response to a preemption. */

   virtual void add(Thread * _thread);
   /* Make the given thread runnable by the scheduler. This function is called
	  typically after thread creation. Depending on the
      implementation, this may not entail more than simply adding the 
      thread to the ready queue (see scheduler_resume). */

   virtual void terminate(Thread * _thread);
   /* Remove the given thread from the scheduler in preparation for destruction
      of the thread. */
};
	
	

#endif