
/* registerrecv.c - register receive */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  registerrecv  - register a receive function  
 *------------------------------------------------------------------------
 */
syscall	registerrecv(umsg32 *abuf, int(*func)(void))
{
	intmask	mask;			/* saved interrupt mask		*/
	struct	procent *prptr;		/* ptr to process' table entry	*/

	mask = disable();
	
	prptr = &proctab[currpid];
	prptr->has_register_recv = TRUE;
	prptr->recv_funcaddr = func;
	prptr->abuf = abuf;

	restore(mask);
	return OK;
}
