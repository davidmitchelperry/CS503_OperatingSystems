Author: David Perry
Date: 3/24/2014
Assignment: CS503 Lab3

Bonus:
	To allow the registered receive function to execute in user space a
	stack frame would need to be created for this function on the
	receiving function's stack. The return adress stored in this stack
	would need to point to the line of code last executed by the receiving
	function. This way when the receiving process was scheduled the
	process would execute the registered function, and then once completed
	with this registered function it would return to where it was
	previously. This not only allows the registered function to be
	executed in userspace but also ensures that the function inherits the
	same privliges as the receiving process.

Added Files:
	
	system/sendb.c:
		Looks through the process entry of the receiving process to
		see if the message buffer is full or empty. If the buffer is
		full the process will be removed from the ready queue and
		placed into the send queue owned by the receiving process and
		calls resched. If the buffer has atleast one spot avialable 
		the message is placed in the buffer and the sending process is
		allowed to continue to run.

	system/receiveb.c:
		If a process calls receive and the message buffer is empty the
		proces will be removed from the ready queue and calls resched.
		If there is a message in the buffer the process reads the
		first message in the buffer, removes the message from the
		buffer, slides other exisiting messages forward into the
		buffer (to ensure that the old message is always read first),
		and the longest (if it exists) waiting process is placed back 
		into the ready queue and resched is called.

	system/registerrecv.c:
		Registers a recieve function that always occurs when the
		registering process is sent a message. This function simply
		sets a flag (indicating a function has been registered) and
		stores a function pointer to the registered function.

	system/getmemgb.c:
		Implements a garbage collected memory allocate. Returns an
		address pointing to where the freed memory begins. However,
		since garbage collection is needed 8 bytes before the returned
		address are used to allocate a memory block. This memory block
		is then linked into a single linked list in the process entry
		structure. This linked list is a memblk list of all memory
		that has been allocated to a process. As a result of this
		method the amount of memory taken from the heap when an
		allocation is rounded(requsted nbytes) + 8. Since getstk uses
		the getmem call rather than getmemgb there should be no
		dependency issues and perfect garbage collection is achieved.

	system/freememgb.c:
		Implements a garbage collected memory free. If the process
		attempts to free a block of memory it owns and gives the
		address given to it from getmemgb the free will be succesful.
		The function looks at the current process's proc entry and
		looks through the memblk linked list structure described in
		getmemgb.c. If it finds the block, it will be freed.
		Otherwise, SYSERR is returned. Again, since freestk uses
		freemem no dependecny issues will occur.

Modified Files:
	system/Makerules:
		Modified to compile the files I added.

	include/process.h:
		Modified to include the the message buffer for sendb, an array
		indicating if a spot in the array is full or empty, and a
		queue for waiting sending processes.

		A boolean of whether or not a receive function has been
		registered, a pointer to that registered function, and a
		pointer to a buffer used by the registered receive function.

		A pointer to a memblk structure that is used to implement a
		linked list for garbage collection.

	include/prototypes.h:
		Added prototypes for:
			receiveb.c
			sendb.c
			registerrecv.b
			getmemgb.c
			freememgb.c

	queue.h:
		Increased the number of queues that can exist to accomodate
		each process having a queue of waiting sending processes.

	system/create.c:
		Modified process creation to initialize all the new values
		added to the procentry struct.

	send.c:
		Modified to handle the case of when a receiving function has
		registered a recieve function. In this case the send function
		executes the resgistered function.

	kill.c:
		Modified to handle the case when a process is killed and has
		processes waiting to send the dieing process a message. In
		this case all processes in the waiting queue are placed back
		into the ready queue.

		Modified to handle the case when a process is killed and has
		allocated memory that was never freed. In this case all the
		memory allocated by the dieing process is freed. This is
		accomplished by walking through the memory reference linked
		list structure in the dieing processes's procentry.
