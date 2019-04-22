/* pipread.c */

#include <xinu.h>
syscall pipread(pipid32 pip, char *buf, uint32 len) {
	
	intmask mask;
	struct pipe *pipeptr;
	int bytes_read;

	mask = disable();

	//Error check pipe id
	if (pip < 0 || pip >= NPIPE) {
		restore(mask);
		return SYSERR;
	}

	//get a ptr to the pipe
	pipeptr = &pipetab[pip];

	//Error check the pipe state
	if(pipeptr->state != PIPE_CONNECTED && pipeptr->state != PIPE_NOWRITER) {
		restore(mask);
		return SYSERR;
	}

	//Make sure the current process is the reader
	if(currpid != pipeptr->reader) {
		restore(mask);
		return SYSERR;
	}

	//if the pipe is empty, wait
	if(semcount(pipeptr->fillcount) <= 0) {
		wait(pipeptr->fillcount);
	}
	//if there are less bytes on the buffer than i was told to read
	if(pipeptr->bytes_on_buffer  <= len) {
		//if i will read to the end of the pipe
		if(pipeptr->readindex + pipeptr->bytes_on_buffer == PIPE_SIZE) {
		       memcpy(buf, &pipeptr->buffer[pipeptr->readindex], pipeptr->bytes_on_buffer);
		       bytes_read = pipeptr->bytes_on_buffer;
		       pipeptr->readindex = 0;
		}
		//if i will read to a spot before the end of the pipe
		else if(pipeptr->readindex + pipeptr->bytes_on_buffer < PIPE_SIZE) {
		       memcpy(buf, &pipeptr->buffer[pipeptr->readindex], pipeptr->bytes_on_buffer);
		       bytes_read = pipeptr->bytes_on_buffer;
		       pipeptr->readindex += pipeptr->bytes_on_buffer;
		}
		//if my read will wrap around the buffer
		else {
		       memcpy(buf, &pipeptr->buffer[pipeptr->readindex], PIPE_SIZE - pipeptr->readindex);
		       memcpy(buf + (PIPE_SIZE - pipeptr->readindex), &pipeptr->buffer[0], (pipeptr->bytes_on_buffer) - (PIPE_SIZE - pipeptr->readindex)); 
		       bytes_read = pipeptr->bytes_on_buffer;
		       pipeptr->readindex = (pipeptr->bytes_on_buffer) - (PIPE_SIZE - pipeptr->readindex);
		}
	}
	//If I'm supposed to read less than all the bytes on the buffer 
	else {
		//If i will read to the end of the pipe
		if(pipeptr->readindex + len == PIPE_SIZE) {
		       memcpy(buf, &pipeptr->buffer[pipeptr->readindex], len);
		       bytes_read = len;
		       pipeptr->readindex = 0;
		}
		//If my read stops before th end of the pipe
		else if(pipeptr->readindex + len < PIPE_SIZE) {
		       memcpy(buf, &pipeptr->buffer[pipeptr->readindex], len);
		       bytes_read = len;
		       pipeptr->readindex += len;
		}
		//if my read wraps around the buffer
		else {
		       memcpy(buf, &pipeptr->buffer[pipeptr->readindex], PIPE_SIZE - pipeptr->readindex);
		       memcpy(buf + (PIPE_SIZE - pipeptr->readindex), &pipeptr->buffer[0], (len) - (PIPE_SIZE - pipeptr->readindex)); 
		       bytes_read = len;
		       pipeptr->readindex = (len) - (PIPE_SIZE - pipeptr->readindex);
		}
	}
	
	//Update the number of bytes of the buffer
	pipeptr->bytes_on_buffer = pipeptr->bytes_on_buffer - bytes_read;
	//Update the fillcount semaphore count
	semtab[pipeptr->fillcount].scount = semcount(pipeptr->fillcount) - bytes_read;
	//reset the the empty count semaphore to the correct value
	semreset(pipeptr->emptycount, PIPE_SIZE - pipeptr->bytes_on_buffer);//PIPE_SIZE - semcount(pipeptr->fillcount));//semcount(pipeptr->emptycount) + bytes_read);

	//if the pipe should be disconnected
	if(pipeptr->state == PIPE_NOWRITER && pipeptr->bytes_on_buffer == 0) {
		pipdisconnect(pip);
	}

	restore(mask);

	return bytes_read;
}
