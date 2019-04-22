/* sendb.c - blocking send */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  sendb  -  pass a message to a process, block if buffer is full, and start recipient if waiting
 *------------------------------------------------------------------------
 */
syscall	sendb(
	  pid32		pid,		/* ID of recipient process	*/
	  umsg32	msg		/* contents of message		*/
	)
{
	intmask	mask;			/* saved interrupt mask		*/
	struct	procent *prptr;		/* ptr to process' table entry	*/
	struct 	procent *currproc; 	/* ptr to current process' table entry DMP */
	int i; 				/* iterator for inserting message in buffer DMP */
	int available_buf_index; 	/* index to available spot in message buffer DMP */

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return SYSERR;
	}

	prptr = &proctab[pid];
	if ((prptr->prstate == PR_FREE)) {
		restore(mask);
		return SYSERR;
	}

	// find an available spot in message buffer if one exists
	available_buf_index = -1;
	for(i = 0; i < MSGBUFSIZE; i++) {
		if(prptr->prmsgstate[i] == MSGEMPTY && available_buf_index == -1) {
			available_buf_index = i;
		}
	}

	// if no spot in buffer is available
	if(available_buf_index == -1) {
		currproc = &proctab[currpid];
		currproc->prstate = PRSND;
		enqueue(currpid, prptr->sendqueue);

		resched();
	
		available_buf_index = -1;
		for(i = 0; i < MSGBUFSIZE; i++) {
			if(prptr->prmsgstate[i] == MSGEMPTY && available_buf_index == -1) {
				available_buf_index = i;
			}
		}
	}

	// Put the message in the message buffer
	prptr->prmsgbuf[available_buf_index] = msg;
	prptr->prmsgstate[available_buf_index] = MSGFULL;

	// If the process is waiting to recieve a message, place it in the ready queue
	if(prptr->prstate == PR_RECVB) {
		ready(pid, RESCHED_YES);
	}

	restore(mask);		/* restore interrupts */
	return OK;
}
