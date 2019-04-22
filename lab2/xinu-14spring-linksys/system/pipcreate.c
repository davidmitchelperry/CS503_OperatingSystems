/* pipcreate.c */

#include <xinu.h>

syscall	pipcreate() {

		int32 		i;
		intmask 	mask;
		struct pipe 	*currpipe;

		//disable interupts
		mask = disable();

		//iterate through pipes
		for (i = 0; i < NPIPE; i++) {
			currpipe = &pipetab[i];
			//check if current pipe is free
			if(currpipe->state == PIPE_FREE) {
			
				// Initialize pipe
				currpipe->state = PIPE_USED;
				currpipe->owner = currpid;
				currpipe->fillcount = semcreate(0);
				currpipe->emptycount = semcreate(PIPE_SIZE);
				currpipe->writeindex = 0;
				currpipe->readindex = 0;
				currpipe->bytes_on_buffer = 0;

				//restore interupts, return pipeid
				restore(mask);
				return currpipe->pipeid;
			}
		}
		//restore interupts, return and error
		restore(mask);
		return SYSERR;
}
			

		


