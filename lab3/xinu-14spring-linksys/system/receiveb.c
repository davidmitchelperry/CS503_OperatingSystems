/* receiveb.c - receiveb */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  receiveb  -  wait for a message and return the message to the caller
 *------------------------------------------------------------------------
 */
umsg32	receiveb(void)
{
	intmask	mask;			/* saved interrupt mask		*/
	struct	procent *prptr;		/* ptr to process' table entry	*/
	umsg32	msg;			/* message to return		*/
	int 	i; 			/* iterator to update message buffer */
	pid32 	send_message; 		/* Pid of process waiting to send a message */

	mask = disable();
	prptr = &proctab[currpid];
	if (prptr->prmsgstate[0] == MSGEMPTY) {
		prptr->prstate = PR_RECVB;
		resched();		/* block until message arrives	*/
	}
	msg = prptr->prmsgbuf[0];		/* retrieve message */
	prptr->prmsgstate[0] = MSGEMPTY; 	/* set the buffer state to empty */

	for(i = 1; i < MSGBUFSIZE; i++) {
		if(prptr->prmsgstate[i] == MSGFULL) {
			prptr->prmsgbuf[i-1] = prptr->prmsgbuf[i];
			prptr->prmsgstate[i-1] = MSGFULL;
			prptr->prmsgstate[i] = MSGEMPTY;
		}
		else {
			break;
		}
	}

	if(nonempty(prptr->sendqueue)) {
		send_message = dequeue(prptr->sendqueue);
		ready(send_message, RESCHED_YES);
	}

	restore(mask);
	return msg;
}
