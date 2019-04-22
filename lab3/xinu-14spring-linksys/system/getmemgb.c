/* getmemgb.c - getmemgb DMP*/

#include <xinu.h>

/*------------------------------------------------------------------------
 *  getmemgb  -  Allocate heap storage, returning lowest word address, and implement garbage collecting
 *------------------------------------------------------------------------
 */
char  	*getmemgb(
	  uint32	nbytes		/* size of memory requested	*/
	)
{
	intmask	mask;			/* saved interrupt mask		*/
	struct	memblk	*prev, *curr, *leftover;
	struct procent *prptr;
	struct memblk *ref_curr;

	mask = disable();
	if (nbytes == 0) {
		restore(mask);
		return (char *)SYSERR;
	}

	nbytes = (uint32) roundmb(nbytes) + 8;	/* use memblk multiples	*/

	prptr = &proctab[currpid]; 		/* get the process entry for currently running process DMP */
	ref_curr = prptr->mem_reference_head;

	prev = &memlist;
	curr = memlist.mnext;
	while (curr != NULL) {			/* search free list	*/

		if (curr->mlength == nbytes) {	/* block is exact match	*/
			prev->mnext = curr->mnext;
			memlist.mlength -= nbytes;

			// Find last memblkgb in reference list
			if(ref_curr != NULL) {
				while(ref_curr->mnext != NULL) {
					ref_curr = ref_curr->mnext;
				}
				curr->mnext = NULL;
				ref_curr->mnext = curr;

			}
			else {
				curr->mnext = NULL;
				prptr->mem_reference_head = curr;
			}

			restore(mask);
			return (char *)(curr + 8);

		} else if (curr->mlength > nbytes) { /* split big block	*/
			leftover = (struct memblk *)((uint32) curr +
					nbytes);
			prev->mnext = leftover;
			leftover->mnext = curr->mnext;
			leftover->mlength = curr->mlength - nbytes;
			memlist.mlength -= nbytes;

			curr->mlength = nbytes;
			
			// Find last memblkgb in reference list
			if(ref_curr != NULL) {
				while(ref_curr->mnext != NULL) {
					ref_curr = ref_curr->mnext;
				}
				curr->mnext = NULL;
				ref_curr->mnext = curr;
			}
			else {

				curr->mnext = NULL;
				prptr->mem_reference_head = curr;
			}
			
			restore(mask);
			return (char *)((uint32)curr + 8);
		} else {			/* move to next block	*/
			prev = curr;
			curr = curr->mnext;
		}
	}
	restore(mask);
	return (char *)SYSERR;
}
