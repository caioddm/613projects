#include "scheduler.H"

Scheduler::Scheduler(){
	queue_size = 0;
	queue = NULL;
	/* Initialize the scheduler with an empty queue */
}

void Scheduler::yield(){	
	if(queue_size > 0){ //check if there is threads ready to run
		Thread* t = queue->thread; //gets the next thread on the queue
		TaskNode* tn = queue;
		queue = queue->next; //removes the first element on the queue
		delete tn;
		queue_size--; //update the number of elements waiting on the queue
		Thread::dispatch_to(t);// run new thread
	}	
}

void Scheduler::enqueue(Thread * _thread){ //puts the thread on the end of the ready queue
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
}

void Scheduler::enqueueBlocked(Thread * _thread){ //the same functionality of the regular enqueue, but to handle the blocked threads queue
	if(blocked_queue_size == 0){
		blockedQueue = new TaskNode(_thread);
	}
	else{ 
		TaskNode* temp = blockedQueue;
		while(temp->next != NULL)
			temp = temp->next; 
			
		temp->next = new TaskNode(_thread);
	}
	blocked_queue_size++;
}

void Scheduler::resume(Thread * _thread){
	enqueue(_thread); //enqueue _thread
	if(blocked_queue_size > 0){ //whenever we are resuming a thread, we will check if there is any I/O blocked thread and put it at the start of of the ready queue to pool it.
		TaskNode* tn = blockedQueue; //gets the next thread on the blocked queue
		blockedQueue = blockedQueue->next; //removes the first element on the queue		
		blocked_queue_size--; //update the number of elements waiting on the blocked queue
		tn->next = queue;
		queue = tn;
		/* make the blocked thread the first on the ready queue */
		queue_size++;
	}
}

void Scheduler::printLists(TaskNode* queue, TaskNode* blockedQueue){
	if(queue_size > 0){
		TaskNode* temp = queue;
		Console::puts("Queue: ");
		Console::putui((unsigned int)temp->thread);
		Console::puts(" ");
		while(temp->next != NULL){
			temp = temp->next; //traverse the list until you reach the end of it
			Console::putui((unsigned int)temp->thread);
			Console::puts(" ");
		}
	}
	if(blocked_queue_size > 0){
		TaskNode* temp = blockedQueue;
		Console::puts("\nBlocked Queue: ");
		Console::putui((unsigned int)temp->thread);
		Console::puts(" ");
		while(temp->next != NULL){
			temp = temp->next; //traverse the list until you reach the end of it
			Console::putui((unsigned int)temp->thread);
			Console::puts(" ");
		}
	}
	Console::puts("\n");
}

void Scheduler::resumeBlocked(Thread * _thread){
	enqueueBlocked(_thread); //enqueue _thread into the blocked thread queue
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