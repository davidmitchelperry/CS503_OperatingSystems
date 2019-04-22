/* pipdisconnect.c */

#include <xinu.h>
syscall pipdisconnect(pipid32 pip) {
	
	intmask mask;
	struct pipe *pipeptr;

	mask = disable();

	//check if pipe id is valid
	if (pip < 0 || pip >= NPIPE) {
		restore(mask);
		return SYSERR;
	}

	//get ptr to pipe
	pipeptr = &pipetab[pip];

	//Error check the pipe state
	if(pipeptr->state != PIPE_CONNECTED && pipeptr->state != PIPE_NOREADER && pipeptr->state != PIPE_NOWRITER) {
		restore(mask);
		return SYSERR;
	}

	//if a non valid process is calling disconnect
	if(currpid != pipeptr->reader && currpid != pipeptr->writer) {
		restore(mask);
		return SYSERR;
	}

	//Error check that the process is not calling disconnect more than once
	if(pipeptr->state == PIPE_NOWRITER && currpid != pipeptr->reader) {
		restore(mask);
		return SYSERR;
	}
	if(pipeptr->state == PIPE_NOREADER && currpid != pipeptr->writer) {
		restore(mask);
		return SYSERR;
	}

	//if pipe is in connected state
	if(pipeptr->state == PIPE_CONNECTED) {
		//if the pid is the writer
		if(currpid == pipeptr->writer) {
			pipeptr->state = PIPE_NOWRITER;
		}
		//if the pid is the reader
		else if(currpid == pipeptr->reader) {
			pipeptr->state = PIPE_NOREADER;
		}
	}
	//If this is the last process to call disconnect
	else if(pipeptr->state == PIPE_NOREADER || pipeptr->state == PIPE_NOWRITER) {
		pipeptr->state = PIPE_USED;
		pipeptr->reader = NULL;
		pipeptr->writer = NULL;
		pipeptr->readindex = 0;
		pipeptr->writeindex = 0;
		memset(pipeptr->buffer, '\0', sizeof(pipeptr->buffer));
		semreset(pipeptr->fillcount, 0);
		semreset(pipeptr->emptycount, PIPE_SIZE);
	}

	restore(mask);

	return OK;
}
