/* e1000Write.c - e1000Write */

#include <xinu.h>

/*------------------------------------------------------------------------
 * e1000Write - write a packet to an E1000E device
 *------------------------------------------------------------------------
 */
devcall	e1000Write(
	struct	dentry	*devptr, 	/* entry in device switch table	*/
	void	*buf,			/* buffer that holds a packet	*/
	uint32	len			/* length of buffer		*/
	)
{
	struct ether *ethptr;
	struct e1000_tx_desc *pkt;
	uint32 TDT;

	ethptr = &ethertab[devptr->dvminor];

	if (ethptr->state != ETH_STATE_UP || len > ETH_BUF_SIZE) {
		return SYSERR;
	}
	
	wait(ethptr->osem);

	pkt = ethptr->txRing + (ethptr->txTail * (sizeof(struct e1000_tx_desc)));

	memset(pkt->buffer_addr, '\0', ETH_BUF_SIZE);
	memcpy(pkt->buffer_addr, buf, len);

	pkt->lower.flags.length = len;
	pkt->lower.flags.cmd = 0x09;

	TDT = (e1000_io_readl(ethptr->iobase, E1000_TDT(0)) + 1) % ethptr->txRingSize;

	e1000_io_writel(ethptr->iobase, E1000_TDT(0), TDT);
	ethptr->txTail = (ethptr->txTail + 1) % ethptr->txRingSize;
	
	
	return 0;
}
