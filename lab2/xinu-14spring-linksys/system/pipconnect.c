/* pipconnect.c */

#include <xinu.h>

syscall pipconnect(pipid32 pip, pid32 writer, pid32 reader) {
	
	intmask mask;
	struct pipe *pipeptr;

	//disable interupts
	mask = disable();

	//Error checking inputs
	if (pip < 0 || pip >= NPIPE || writer == reader || isbadpid(reader) || isbadpid(writer)) {
		restore(mask);
		return SYSERR;
	}
	//get a ptr to the pipe
	pipeptr = &pipetab[pip];

	//if the pipe is not in used state, restore interupts, return error
	if(pipeptr->state != PIPE_USED) {
		restore(mask);
		return SYSERR;
	}

	//set the correct state, reader, and writer
	pipeptr->state = PIPE_CONNECTED;
	pipeptr->reader = reader;
	pipeptr->writer = writer;

	restore(mask);

	return OK;
}



