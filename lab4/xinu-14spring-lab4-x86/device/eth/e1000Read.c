/* e1000Read.c - e1000Read */

#include <xinu.h>

/*------------------------------------------------------------------------
 * e1000Read - read a packet from an E1000E device
 *------------------------------------------------------------------------
 */
devcall	e1000Read(
	struct	dentry	*devptr,	/* entry in device switch table	*/
	void	*buf,			/* buffer to hold packet	*/
	uint32	len			/* length of buffer		*/
	)
{
	struct ether *ethptr;
	struct e1000_rx_desc * rx_desc;
	int toCpy = 0;
	uint32 RDT;

	ethptr = &ethertab[devptr->dvminor];
	wait(ethptr->isem);
	rx_desc = ethptr->rxRing + (ethptr->rxHead * (sizeof(struct e1000_rx_desc)));
	
	if(rx_desc->length < len) {
		toCpy = rx_desc->length;
	}
	else {
		toCpy = len;
	}

	memcpy(buf, rx_desc->buffer_addr, rx_desc->length);

	RDT = e1000_io_readl(ethptr->iobase, E1000_RDT(0));
	RDT += 1;
	RDT %= ethptr->rxRingSize;

	memset(rx_desc->buffer_addr, '\0', ETH_BUF_SIZE);
	rx_desc->length = 0;
	rx_desc->csum = 0;
	rx_desc->status = 0;
	rx_desc->errors = 0;
	rx_desc->special = 0;

	e1000_io_writel(ethptr->iobase, E1000_RDT(0), RDT);
	ethptr->rxHead += 1;
	ethptr->rxHead %= ethptr->rxRingSize;


	return 0;
}
