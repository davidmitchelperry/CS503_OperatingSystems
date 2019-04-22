/* freememgb.c - garbage collecting version of freemem */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  freememgb  -  Free a memory block, returning the block to the free list
 *------------------------------------------------------------------------
 */
syscall	freememgb(
	  char		*blkaddr,	/* pointer to memory block	*/
	  uint32	nbytes		/* size of block in bytes	*/
	)
{
	intmask	mask;			/* saved interrupt mask		*/
	struct	memblk	*next, *prev, *block;
	uint32	top;
	struct procent *prptr;
	struct memblk *curr_ref;
	struct memblk *prev_ref, *new_last_block;

	mask = disable();
	if ((nbytes == 0) || ((uint32) blkaddr < (uint32) minheap)
			  || ((uint32) blkaddr > (uint32) maxheap)) {
		restore(mask);
		return SYSERR;
	}
	
	prptr = &proctab[currpid];
	if(prptr->mem_reference_head == NULL) {
		restore(mask);
		return SYSERR;
	}

	curr_ref = prptr->mem_reference_head;
	prev_ref = NULL;
	new_last_block = NULL;
	block = NULL;
	while(curr_ref != NULL) {
		if((uint32)curr_ref == ((uint32)blkaddr - 8)) {
			block = curr_ref;
			new_last_block = prev_ref;
		}
		prev_ref = curr_ref;
		curr_ref = curr_ref->mnext;
	}

	// Process did not allocate the block
	if(block == NULL) {
		restore(mask);
		return SYSERR;
	}

	if(new_last_block == NULL) {
		if(block->mnext == NULL) {
			prptr->mem_reference_head = NULL;
		}
		else {
			prptr->mem_reference_head = block->mnext;
		}
	}
	else {
		if(block->mnext == NULL) {
			new_last_block->mnext = NULL;
		}
		else {
			new_last_block->mnext = block->mnext;
		}
	}

	nbytes = block->mlength;

	//nbytes = (uint32) roundmb(nbytes);	/* use memblk multiples	*/
	//block = (struct memblk *)blkaddr;

	prev = &memlist;			/* walk along free list	*/
	next = memlist.mnext;
	while ((next != NULL) && (next < block)) {
		prev = next;
		next = next->mnext;
	}

	if (prev == &memlist) {		/* compute top of previous block*/
		top = (uint32) NULL;
	} else {
		top = (uint32) prev + prev->mlength;
	}

	/* Insure new block does not overlap previous or next blocks	*/

	if (((prev != &memlist) && (uint32) block < top)
	    || ((next != NULL)	&& (uint32) block+nbytes>(uint32)next)) {
		restore(mask);
		return SYSERR;
	}

	memlist.mlength += nbytes;

	/* Either coalesce with previous block or add to free list */

	if (top == (uint32) block) { 	/* coalesce with previous block	*/
		prev->mlength += nbytes;
		block = prev;
	} else {			/* link into list as new node	*/
		block->mnext = next;
		block->mlength = nbytes;
		prev->mnext = block;
	}

	/* Coalesce with next block if adjacent */

	if (((uint32) block + block->mlength) == (uint32) next) {
		block->mlength += next->mlength;
		block->mnext = next->mnext;
	}
	restore(mask);
	return OK;
}
