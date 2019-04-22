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
	struct memblk *ref_curr;

	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)
	    || ((prptr = &proctab[pid])->prstate) == PR_FREE) {
		restore(mask);
		return SYSERR;
	}
	/* If there are process waiting to send to killed process DMP */
	if(nonempty(prptr->sendqueue)) {
		/* Free the waiting processes DMP */
		while(nonempty(prptr->sendqueue)) {
			ready(dequeue(prptr->sendqueue), RESCHED_NO);
		}
	}

	/* Free all memory owned by the process being killed DMP */
	ref_curr = prptr->mem_reference_head;
	while(ref_curr != NULL) {
		freememgb((char *)(uint32)(ref_curr) + 8, ref_curr->mlength - 8);
		ref_curr = prptr->mem_reference_head;
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
