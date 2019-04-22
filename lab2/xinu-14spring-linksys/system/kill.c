/* kill.c - kill */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  kill  -  Kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
syscall	kill(
	  pid32		pid		/* ID of process to kill	*/
	)
{

	intmask	mask;			/* saved interrupt mask		*/
	struct	procent *prptr;		/* ptr to process' table entry	*/
	int32	i;			/* index into descriptors	*/
	int32 	j; 			/* index into the pipe table DMP*/
	struct pipe *pipeptr; 		/* ptr to pipe in table DMP 	*/

	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)
	    || ((prptr = &proctab[pid])->prstate) == PR_FREE) {
		restore(mask);
		return SYSERR;
	}

	/* For Pipe Deletion and Disconnecting DMP */
	for(j = 0; j < NPIPE; j++) {
		pipeptr = &pipetab[j];
		//if the ownder of the pipe is being killed
		if(pipeptr->owner == pid) {
			pipeptr->state = PIPE_FREE;
			memset(pipeptr->buffer, '\0', sizeof(pipeptr->buffer));
			pipeptr->owner = NULL;
			pipeptr->reader = NULL;
			pipeptr->writer = NULL;
			pipeptr->readindex = 0;
			pipeptr->writeindex = 0;
			pipeptr->bytes_on_buffer = 0;
			semdelete(pipeptr->fillcount);
			semdelete(pipeptr->emptycount);
		}
		//if the writer or reader of a pipe is being deleted
		if(pipeptr->writer == pid || pipeptr->reader == pid) {
			if(pipeptr->state == PIPE_NOWRITER) {
				if(pid == pipeptr->reader) {
					pipeptr->state = PIPE_USED;
					pipeptr->reader = NULL;
					pipeptr->writer = NULL;
					pipeptr->readindex = 0;
					pipeptr->writeindex = 0;
					pipeptr->bytes_on_buffer = 0;
					memset(pipeptr->buffer, '\0', sizeof(pipeptr->buffer));
					semreset(pipeptr->fillcount, 0);
					semreset(pipeptr->emptycount, PIPE_SIZE);
				}
					
			}
			//if the pipe has been disconnected from the reader
			else if(pipeptr->state == PIPE_NOREADER) {
				if(pid == pipeptr->writer) {
					pipeptr->state = PIPE_USED;
					pipeptr->reader = NULL;
					pipeptr->writer = NULL;
					pipeptr->readindex = 0;
					pipeptr->writeindex = 0;
					pipeptr->bytes_on_buffer = 0;
					memset(pipeptr->buffer, '\0', sizeof(pipeptr->buffer));
					semreset(pipeptr->fillcount, 0);
					semreset(pipeptr->emptycount, PIPE_SIZE);
				}
			}
			
			else if(pipeptr->state == PIPE_CONNECTED) {
				if(pipeptr->writer == pid) {
					pipeptr->state = PIPE_NOWRITER;
				}
				if(pipeptr->reader == pid) {
					pipeptr->state = PIPE_NOREADER;
				}
			}
		}
	}

	if (--prcount <= 1) {		/* last user process completes	*/
		xdone();
	}

	send(prptr->prparent, pid);
	for (i=0; i<3; i++) {
		close(prptr->prdesc[i]);
	}
	freestk(prptr->prstkbase, prptr->prstklen);

	switch (prptr->prstate) {
	case PR_CURR:
		prptr->prstate = PR_FREE;	/* suicide */
		resched();

	case PR_SLEEP:
	case PR_RECTIM:
		unsleep(pid);
		prptr->prstate = PR_FREE;
		break;

	case PR_WAIT:
		semtab[prptr->prsem].scount++;
		/* fall through */

	case PR_READY:
		getitem(pid);		/* remove from queue */
		/* fall through */

	default:
		prptr->prstate = PR_FREE;
	}

	restore(mask);
	return OK;
}
