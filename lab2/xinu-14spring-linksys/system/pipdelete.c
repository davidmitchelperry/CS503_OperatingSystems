/* pipdelete.c */

#include <xinu.h>
#include <string.h>

syscall pipdelete(pipid32 pip) {

	intmask mask;
	struct pipe *pipeptr;

	mask = disable();
	//Error check pipe id
	if (pip < 0 || pip >= NPIPE) {
		restore(mask);
		return SYSERR;
	}
	//get a ptr to the pipe
	pipeptr = &pipetab[pip];

	//if the caller is not the owner
	if(pipeptr->owner != currpid) {
		restore(mask);
		return SYSERR;
	}


	//Completely clear pipe and relinquish ownership
	pipeptr->state = PIPE_FREE;
	memset(pipeptr->buffer, '\0', sizeof(pipeptr->buffer));
	pipeptr->owner = NULL;
	pipeptr->reader = NULL;
	pipeptr->writer = NULL;
	pipeptr->readindex = 0;
	pipeptr->writeindex = 0;
	semdelete(pipeptr->fillcount);
	semdelete(pipeptr->emptycount);

	restore(mask);
	return OK;
}
