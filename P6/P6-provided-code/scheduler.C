#include "scheduler.H"

Scheduler::Scheduler(){
	queue_size = 0;
	queue = NULL;
	/* Initialize the scheduler with an empty queue */
}

void Scheduler::yield(){
	if(queue_size > 0){ //check if there is threads ready to run
		//if(machine_interrupts_enabled()) //disable interrupts prior to a context switch
        //	machine_disable_interrupts();
		Thread* t = queue->thread; //gets the next thread on the queue
		TaskNode* tn = queue;
		queue = queue->next; //removes the first element on the queue
		delete tn;
		queue_size--; //update the number of elements waiting on the queue
		//if(!machine_interrupts_enabled())
        //	machine_enable_interrupts();
		Thread::dispatch_to(t);// run new thread
	}	
}

void Scheduler::enqueue(Thread * _thread){ //puts the thread on the end of the ready queue
	//if(machine_interrupts_enabled()) //disable interrupts prior to a context switch
    //    	machine_disable_interrupts();
	if(queue_size == 0){
		queue = new TaskNode(_thread); //queue is empty, so simply make the first element of the queue be _thread
	}
	else{ //queue is not empty, enqueue _thread to the end of the ready list		
		TaskNode* temp = queue;
		while(temp->next != NULL)
			temp = temp->next; //traverse the list until you reach the end of it
			
		temp->next = new TaskNode(_thread); //put _thread at the end of the list
	}
	queue_size++; //increment the size of the queue
	//if(!machine_interrupts_enabled())
    //    	machine_enable_interrupts();
}

void Scheduler::resume(Thread * _thread){
	enqueue(_thread); //enqueue _thread
}

void Scheduler::add(Thread * _thread){
	enqueue(_thread); //enqueue _thread
}

void Scheduler::terminate(Thread * _thread){
	

	if(queue_size > 0){
		TaskNode* temp = queue;
		if(temp->thread->ThreadId()==_thread->ThreadId()){ //task to terminate is the first in the list
			queue = temp->next;
			delete temp; //release the memory used by the tasknode on the queue
			delete _thread; //release the memory used by the terminated thread
			queue_size--;
		}
		else{ //traverse the rest of the list looking for the thread to terminate
			while(temp->next != NULL){
				if (temp->next->thread->ThreadId()==_thread->ThreadId()){
					TaskNode* tn = temp->next;
					temp->next = tn->next;
					delete tn; //release the memory used by the tasknode on the queue
					delete _thread; //release the memory used by the terminated thread
					queue_size--;
					break;
				}
				temp = temp->next;
			}
		}	
	}
	yield();
	
}