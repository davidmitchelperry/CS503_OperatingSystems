/* pipwrite.c */

#include <xinu.h>
syscall pipwrite(pipid32 pip, char *buf, uint32 len) {
	
	intmask mask;
	struct pipe *pipeptr;
	int i;//, j;
	int prev_i;
	int bytes_written;

	mask = disable();

	//Error check pipe id
	if (pip < 0 || pip >= NPIPE) {
		restore(mask);
		return SYSERR;
	}

	pipeptr = &pipetab[pip];

	//error check pipe state
	if(pipeptr->state != PIPE_CONNECTED && pipeptr->state != PIPE_NOREADER) {
		restore(mask);
		return SYSERR;
	}

	//make sure the caller is the writer of  process
	if(currpid != pipeptr->writer) {
		restore(mask);
		return SYSERR;
	}
	//while a write still needs to occur
	i = 0;
	while(i < len) {
	
		//Check the state of the pipe again
		if(pipeptr->state != PIPE_CONNECTED && pipeptr->state != PIPE_NOREADER) {
			restore(mask);
			return SYSERR;
		}
		//Check the owner of the pipe again
		if(currpid != pipeptr->writer) {
			restore(mask);
			return SYSERR;
		}
		//block if the pipe has no available space to write	
		wait(pipeptr->emptycount);

		//Reperform state check after wait
		if(pipeptr->state != PIPE_CONNECTED && pipeptr->state != PIPE_NOREADER) {
			restore(mask);
			return SYSERR;
		}
		//Reperform writer check
		if(currpid != pipeptr->writer) {
			restore(mask);
			return SYSERR;
		}
		//if everything that needs to be written can currently fit on the buffer
		if((len - i) <= PIPE_SIZE - pipeptr->bytes_on_buffer) {
			//if what needs to be written will land on the last spot of the buffer
			if(pipeptr->writeindex + (len - i) == PIPE_SIZE) {
				memcpy(&pipeptr->buffer[pipeptr->writeindex], &buf[i], (len - i));
				bytes_written = (len - i);
				pipeptr->writeindex = 0;
				i += (len - i);
			}
			//if what needs to be written lands on an index before the last index
			else if(pipeptr->writeindex + (len - i) < PIPE_SIZE) {
				memcpy(&pipeptr->buffer[pipeptr->writeindex], &buf[i], (len - i));
				bytes_written = (len - i);
				pipeptr->writeindex += (len - i);
				i += (len - i);
			}
			//if what needs to be written will wrap around the pipe
			else {
				memcpy(&pipeptr->buffer[pipeptr->writeindex], &buf[i], PIPE_SIZE - pipeptr->writeindex);
				prev_i = i;
				i += PIPE_SIZE - pipeptr->writeindex;
				bytes_written = PIPE_SIZE - pipeptr->writeindex;
				memcpy(&pipeptr->buffer[0], &buf[i], (len - prev_i) - (PIPE_SIZE - pipeptr->writeindex)); 
				i += (len - prev_i) - (PIPE_SIZE - pipeptr->writeindex);
				bytes_written += (len - prev_i) - (PIPE_SIZE - pipeptr->writeindex);
				pipeptr->writeindex = (len - prev_i) - (PIPE_SIZE - pipeptr->writeindex);
			}

		}
		//if more writes will need to occur
		else {
			//if what needs to be written will land on the last spot of the buffer
			if(pipeptr->writeindex + (PIPE_SIZE - pipeptr->bytes_on_buffer) == PIPE_SIZE) {
				memcpy(&pipeptr->buffer[pipeptr->writeindex], &buf[i], PIPE_SIZE - pipeptr->bytes_on_buffer);
				bytes_written = PIPE_SIZE - pipeptr->bytes_on_buffer;
				pipeptr->writeindex = 0;
				i += PIPE_SIZE - pipeptr->bytes_on_buffer;
			}
			//if what needs to be written lands on an index before the last index
			else if(pipeptr->writeindex + (PIPE_SIZE - pipeptr->bytes_on_buffer) < PIPE_SIZE) {
				memcpy(&pipeptr->buffer[pipeptr->writeindex], &buf[i], PIPE_SIZE - pipeptr->bytes_on_buffer);
				bytes_written = PIPE_SIZE - pipeptr->bytes_on_buffer;
				pipeptr->writeindex += (PIPE_SIZE - pipeptr->bytes_on_buffer);
				i += PIPE_SIZE - pipeptr->bytes_on_buffer;
			}
			//if what needs to be written will wrap around the pipe
			else {
				memcpy(&pipeptr->buffer[pipeptr->writeindex], &buf[i], PIPE_SIZE - pipeptr->writeindex);
				i += PIPE_SIZE - pipeptr->writeindex;
				memcpy(&pipeptr->buffer[0], &buf[i], (PIPE_SIZE - pipeptr->bytes_on_buffer) - (PIPE_SIZE - pipeptr->writeindex)); 
				i += (PIPE_SIZE - pipeptr->bytes_on_buffer) - (PIPE_SIZE - pipeptr->writeindex);
				bytes_written = (PIPE_SIZE - pipeptr->writeindex) + (PIPE_SIZE - pipeptr->bytes_on_buffer) - (PIPE_SIZE - pipeptr->writeindex);
				pipeptr->writeindex = (PIPE_SIZE - pipeptr->bytes_on_buffer) - (PIPE_SIZE - pipeptr->writeindex);
			}
		}

		//update the number of bytes on the buffer
		pipeptr->bytes_on_buffer +=bytes_written; 

		//update semaphores and signal fillcount for each byte written
		semtab[pipeptr->emptycount].scount = (semtab[pipeptr->emptycount].scount - bytes_written);
		signaln(pipeptr->fillcount, bytes_written);
		//Check if the reader has disconnected
		if(pipeptr->state == PIPE_NOREADER) {
			pipdisconnect(pip);
		}

	}

	//check if the reader has disconnected
	if(pipeptr->state == PIPE_NOREADER) {
		pipdisconnect(pip);
	}

	restore(mask);

	return OK;
}
