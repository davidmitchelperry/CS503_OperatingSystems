/* 82545EMInit.c - _82545EMInit */

#include <xinu.h>

local 	status 	_82545EM_init_hw(struct ether *);
local 	void 	_82545EM_reset_hw(struct ether *);
local 	void 	_82545EM_configure_rx(struct ether *);
local 	void 	_82545EM_configure_tx(struct ether *);
status 	_82545EM_read_phy_reg(struct ether *, uint32, uint16 *);
status 	_82545EM_write_phy_reg(struct ether *, uint32, uint16);


/*------------------------------------------------------------------------
 * _82545EMInit - initialize Intel 82545EM Ethernet NIC
 *------------------------------------------------------------------------
 */
status 	_82545EMInit(
	struct 	ether *ethptr
	)
{
	int i;
	struct e1000_rx_desc *rx_desc; 
	struct e1000_tx_desc *tx_desc;

	/* Read PCI configuration information */
	pci_bios_read_config_dword(ethptr->pcidev, E1000_PCI_IOBASE ,  &ethptr->iobase);

	/* Read I/O base address */
	ethptr->iobase &= 0xfffffffc;

	/* Read interrupt line number */
	pci_bios_read_config_byte(ethptr->pcidev, E1000_PCI_IRQ, &(ethptr->dev->dvirq));

	/* Enable PCI bus master, I/O port access */
	pci_bios_write_config_byte(ethptr->pcidev, E1000_PCI_COMMAND, 0x7);

	/* Read the MAC address */
	ethptr->RAL = e1000_io_readl(ethptr->iobase, E1000_RAL(0));
	ethptr->RAH = e1000_io_readl(ethptr->iobase, E1000_RAH(0));

	for(i = 0; i < ETH_ADDR_LEN; i++) {
		ethptr->devAddress[i]= (byte)(ethptr->RAL >> (i * 8));
	}
	for(i = 0; i < ETH_ADDR_LEN; i++) {
		ethptr->devAddress[i + 4]= (byte)(ethptr->RAH >> (i * 8));
	}
	

	/* Initialize structure pointers */
	ethptr->rxRingSize = E1000_RX_RING_SIZE;
	ethptr->rxIrq = 0;

	ethptr->txRingSize = E1000_TX_RING_SIZE;
	ethptr->txIrq = 0;

	ethptr->addrLen = ETH_ADDR_LEN;
	ethptr->mtu = ETH_MTU;
	ethptr->errors = 0;
	ethptr->isem = semcreate(0);
	ethptr->osem = semcreate(E1000_TX_RING_SIZE);
	ethptr->proms = 0;

	/* Rings must be aligned on a 16-byte boundary */
	ethptr->rxRing = getmem(ethptr->rxRingSize * sizeof(struct e1000_rx_desc));
	ethptr->rxBufs = getmem(ethptr->rxRingSize * ETH_BUF_SIZE);
	ethptr->rxHead = 0;
	ethptr->rxTail = 0;
	
	ethptr->txRing = getmem(ethptr->txRingSize * sizeof(struct e1000_tx_desc));
	ethptr->txBufs = getmem(ethptr->txRingSize * ETH_BUF_SIZE);
	ethptr->txHead = 0;
	ethptr->txTail = 0;
	
	/* Buffers are highly recommended to be allocated on cache-line */
	/* 	size (64-byte for E8400) 				*/
	

	/* Set buffer pointers and rings to zero */
	memset(ethptr->rxRing, '\0', ethptr->rxRingSize * sizeof(struct e1000_rx_desc));
	memset(ethptr->rxBufs, '\0', ethptr->rxRingSize * ETH_BUF_SIZE);
	memset(ethptr->txRing, '\0', ethptr->txRingSize * sizeof(struct e1000_tx_desc));
	memset(ethptr->txBufs, '\0', ethptr->txRingSize * ETH_BUF_SIZE);

	/* Insert the buffer into descriptor ring */
	for(i = 0; i < ethptr->rxRingSize; i++) {
		rx_desc= ethptr->rxRing + (i * sizeof(struct e1000_rx_desc));
		rx_desc->buffer_addr = (uint32)ethptr->rxBufs + (i * ETH_BUF_SIZE);
	}
	for(i = 0; i < ethptr->txRingSize; i++) {
		tx_desc= ethptr->txRing + (i * sizeof(struct e1000_tx_desc));
		tx_desc->buffer_addr = (uint32)ethptr->txBufs + (i * ETH_BUF_SIZE);
	}


	/* Reset packet buffer allocation to default */
	e1000_io_writel(ethptr->iobase, E1000_PBA, E1000_PBA_48K);

	/* Reset the NIC to bring it into a known state and initialize it */
	_82545EM_reset_hw(ethptr);

	/* Initialize the hardware */
	if (_82545EM_init_hw(ethptr) != OK)
		return SYSERR;

	/* Configure the NIC */
	e1000_io_writel(ethptr->iobase, E1000_AIT, 0);

	/* Configure the RX */
	_82545EM_configure_rx(ethptr);
	
	/* Configure the TX */
	_82545EM_configure_tx(ethptr);

	/* Register the interrupt and enable interrupt */
	set_evec(ethptr->dev->dvirq + IRQBASE, (uint32)e1000Dispatch);
	e1000IrqEnable(ethptr);

	return OK;
}

/*------------------------------------------------------------------------
 * _82545EM_reset_hw - Reset the hardware 
 *------------------------------------------------------------------------
 */
local void _82545EM_reset_hw(
	struct 	ether *ethptr
	)
{
	/* Masking off all interrupts */
	e1000IrqDisable(ethptr);

	/* Disable the Transmit and Receive units. */
	e1000_io_writel(ethptr->iobase, E1000_TCTL, 0x00000000);
	e1000_io_writel(ethptr->iobase, E1000_RCTL, 0x00000000);

	/* Issuing a global reset by setting CTRL register with E1000_CTRL_RST*/
	e1000_io_writel(ethptr->iobase, E1000_CTRL, E1000_CTRL_RST);
	
    /* Delay slightly to let hardware process */
    	DELAY(50);

    /* Masking off all interrupts again*/
    	e1000IrqDisable(ethptr);

}

/*------------------------------------------------------------------------
 * _82545EM_init_hw - Initialize the hardware
 *------------------------------------------------------------------------
 */
local status _82545EM_init_hw(
	struct 	ether *ethptr
	)
{
	int i;
	uint16 PHY_AUTONEG_ADV;
	uint16 PHY_CONTROL;
	uint32 CTRL;
       	int status = 0;
       	int ret_status = SYSERR;

	/* Setup the receive address */
	e1000_io_writel(ethptr->iobase, E1000_RAL(0), ethptr->RAL);
	e1000_io_writel(ethptr->iobase, E1000_RAH(0), ethptr->RAH);
	/* Zero out the other receive addresses */
	for(i = 1; i < 16; i++) {
		e1000_io_writel(ethptr->iobase, E1000_RAL(i), 0);
		e1000_io_writel(ethptr->iobase, E1000_RAH(i), 0);
	}

	/* Zero out the Multicast HASH table */
	for(i = 0; i < E1000_82545EM_MTA_ENTRIES; i++) {
		e1000_io_writel(ethptr->iobase, (i * 4) + E1000_MTA, 0);
	}
	/* Commit the changes.*/
	pci_bios_write_config_byte(ethptr->pcidev, E1000_PCI_COMMAND, 0xff);

    /* Do a slightly delay for the hardware to proceed the commit */
    	DELAY(50);

	/* Setup autoneg and flow control advertisement and perform 	*/
	/* 	autonegotiation. 					*/
	_82545EM_read_phy_reg(ethptr, E1000_PHY_AUTONEG_ADV, &PHY_AUTONEG_ADV);
	PHY_AUTONEG_ADV |= E1000_CR_1000T_FD_CAPS | E1000_CR_1000T_HD_CAPS 
			| E1000_NWAY_AR_ASM_DIR | E1000_NWAY_AR_PAUSE 
			| E1000_NWAY_AR_100TX_FD_CAPS | E1000_NWAY_AR_10T_HD_CAPS
			| E1000_NWAY_AR_10T_FD_CAPS | E1000_NWAY_AR_10T_HD_CAPS;
	//e1000_io_writel(ethptr->iobase, E1000_CTRL, E1000_CTRL_RFCE | E1000_CTRL_TFCE);
	_82545EM_write_phy_reg(ethptr, E1000_PHY_AUTONEG_ADV, PHY_AUTONEG_ADV);


	/* Restart auto-negotiation. */
	_82545EM_read_phy_reg(ethptr, E1000_PHY_CONTROL, &PHY_CONTROL);
	_82545EM_write_phy_reg(ethptr, E1000_PHY_CONTROL, 
		PHY_CONTROL | E1000_MII_CR_RESTART_AUTO_NEG 
		| E1000_MII_CR_AUTO_NEG_EN);


	/* Wait for auto-negotiation to complete 
       Implement a loop here to check the E1000_MII_SR_LINK_STATUS and E1000_MII_SR_AUTONEG_COMPLETE, break if they are both ture
       You should also delay for a while in each loop so it won't take too much CPU time */


	for(i = 0; i < 15; i++) {
		_82545EM_read_phy_reg(ethptr, E1000_PHY_STATUS, &status);
		_82545EM_read_phy_reg(ethptr, E1000_PHY_STATUS, &status);
		if((status & E1000_MII_SR_LINK_STATUS) && (status & E1000_MII_SR_AUTONEG_COMPLETE)) {
			ret_status = OK;
			break;
		}
		status = 0;
		DELAY(50);
	}
       		

    /* Update device control according receive flow control and transmit flow control*/
    	CTRL = e1000_io_readl(ethptr->iobase, E1000_CTRL);
    	e1000_io_writel(ethptr->iobase, E1000_CTRL, E1000_CTRL_RFCE | E1000_CTRL_TFCE);
	CTRL &= ~E1000_CTRL_FRCDPX;
	CTRL &= ~E1000_CTRL_FRCSPD;
	CTRL |= E1000_CTRL_SLU;
	e1000_io_writel(ethptr->iobase, E1000_CTRL, CTRL);

	return ret_status;
}

/*------------------------------------------------------------------------
 * _82545EM_configure_rx - Configure Receive Unit after Reset
 *------------------------------------------------------------------------
 */
local void _82545EM_configure_rx(
	struct 	ether *ethptr
	)
{


	/* Disable receiver while configuring. */
	e1000_io_writel(ethptr->iobase, E1000_RCTL, 0x00000000);


	/* Enable receiver, accept broadcast packets, no loopback, and 	*/
	/* 	free buffer threshold is set to 1/2 RDLEN. 		*/


	/* Do not store bad packets, do not pass MAC control frame, 	*/
	/* 	disable long packet receive and CRC strip 		*/
	
	
	/* Setup buffer sizes */


	/* Set the Receive Delay Timer Register, let driver be notified */
	/* 	immediately each time a new packet has been stored in 	*/
	/* 	memory 							*/
	e1000_io_writel(ethptr->iobase, E1000_RDTR, E1000_RDTR_DEFAULT);
	e1000_io_writel(ethptr->iobase, E1000_RADV, E1000_RADV_DEFAULT);


	/* Set up interrupt rate to be default. Notice that it is a the rate is not just E1000_ITR_DEFAULT which is the frequency, 
       it is 1000000000 / (E1000_ITR_DEFAULT * 256) */
       e1000_io_writel(ethptr->iobase, E1000_ITR, 1000000000 / (E1000_ITR_DEFAULT * 256));

	/* Setup the HW Rx Head and Tail Descriptor Pointers, the Base 	*/
	/* 	and Length of the Rx Descriptor Ring 			*/
	e1000_io_writel(ethptr->iobase, E1000_RDH(0), 0);
	e1000_io_writel(ethptr->iobase, E1000_RDT(0), 1);
	e1000_io_writel(ethptr->iobase, E1000_RDBAL(0), ethptr->rxRing);
	e1000_io_writel(ethptr->iobase, E1000_RDBAH(0), 0);
	e1000_io_writel(ethptr->iobase, E1000_RDLEN(0), (ethptr->rxRingSize * sizeof(struct e1000_rx_desc)));

	/* Disable Receive Checksum Offload for IPv4, TCP and UDP. */
	e1000_io_writel(ethptr->iobase, E1000_RXCSUM, 0x00000000);

	/* Enable receiver. */
	e1000_io_writel(ethptr->iobase, E1000_RCTL, E1000_RCTL_EN | E1000_RCTL_BAM);

}

/*------------------------------------------------------------------------
 * _82545EM_configure_tx - Configure Transmit Unit after Reset
 *------------------------------------------------------------------------
 */
local void _82545EM_configure_tx(
	struct 	ether *ethptr
	)
{
	uint32 TCTL;

	/* Set the transmit descriptor write-back policy for both queues */
	e1000_io_writel(ethptr->iobase, E1000_TXDCTL(0), E1000_TXDCTL_WTHRESH | 
							 E1000_TXDCTL_GRAN | 
							 E1000_TXDCTL_MAX_TX_DESC_PREFETCH);

	/* Program the Transmit Control Register */
	TCTL = e1000_io_readl(ethptr->iobase, E1000_TCTL);


	/* Set the default values for the Tx Inter Packet Gap timer */
	e1000_io_writel(ethptr->iobase, E1000_TIPG, E1000_TIPG_IPGT_COPPER_DEFAULT | 
						    (E1000_TIPG_IPGR1_DEFAULT << 
						    E1000_TIPG_IPGR1_SHIFT) |
						    (E1000_TIPG_IPGR2_DEFAULT << 
						    E1000_TIPG_IPGR2_SHIFT));


	/* Set the Tx Interrupt Delay register */
	e1000_io_writel(ethptr->iobase, E1000_TIDV, E1000_TIDV_DEFAULT);

	/* Setup the HW Tx Head and Tail descriptor pointers */
	e1000_io_writel(ethptr->iobase, E1000_TDBAL(0), ethptr->txRing);
	e1000_io_writel(ethptr->iobase, E1000_TDBAH(0), 0);
	e1000_io_writel(ethptr->iobase, E1000_TDLEN(0), (ethptr->txRingSize * sizeof(struct e1000_tx_desc)) << 7);	
	e1000_io_writel(ethptr->iobase, E1000_TDH(0), 0);
	e1000_io_writel(ethptr->iobase, E1000_TDT(0), 0);


    /* Enable transmit but setting TCTL*/
    TCTL |= E1000_TCTL_RTLC | E1000_TCTL_COLD | E1000_TCTL_CT | E1000_TCTL_PSP | E1000_TCTL_EN;
    e1000_io_writel(ethptr->iobase, E1000_TCTL, TCTL);
}

/*------------------------------------------------------------------------
 * _82545EM_read_phy_reg - Read MDI control register
 *------------------------------------------------------------------------
 */
status _82545EM_read_phy_reg(
	struct 	ether *ethptr,
	uint32 	offset,
	uint16 	*data
	)
{
	uint32 i, mdic = 0;

	if (offset > E1000_MAX_PHY_REG_ADDRESS) {
		return SYSERR;
	}

	mdic = ((offset << E1000_MDIC_REG_SHIFT) |
		(E1000_82545EM_MDIC_PHY_ADDR << E1000_MDIC_PHY_SHIFT) |
		(E1000_MDIC_OP_READ));

	e1000_io_writel(ethptr->iobase, E1000_MDIC, mdic);

	for (i = 0; i < (E1000_GEN_POLL_TIMEOUT * 3); i++) {
		DELAY(50);
		mdic = e1000_io_readl(ethptr->iobase, E1000_MDIC);
		if (mdic & E1000_MDIC_READY)
			break;
	}
	if (!(mdic & E1000_MDIC_READY)) {
		return SYSERR;
	}
	if (mdic & E1000_MDIC_ERROR) {
		return SYSERR;
	}
	*data = (uint16) mdic;

	return OK;
}

/*------------------------------------------------------------------------
 *  _82545EM_write_phy_reg - Write MDI control register
 *------------------------------------------------------------------------
 */
status _82545EM_write_phy_reg(
	struct 	ether *ethptr,
	uint32 	offset,
	uint16 	data
	)
{
	uint32 i, mdic = 0;

	if (offset > E1000_MAX_PHY_REG_ADDRESS) {
		return SYSERR;
	}

	mdic = ( ((uint32)data) |
		 (offset << E1000_MDIC_REG_SHIFT) |
		 (E1000_82545EM_MDIC_PHY_ADDR << E1000_MDIC_PHY_SHIFT) |
		 (E1000_MDIC_OP_WRITE) );

	e1000_io_writel(ethptr->iobase, E1000_MDIC, mdic);

	for (i = 0; i < (E1000_GEN_POLL_TIMEOUT * 3); i++) {
		DELAY(50);
		mdic = e1000_io_readl(ethptr->iobase, E1000_MDIC);
		if (mdic & E1000_MDIC_READY)
			break;
	}
	if (!(mdic & E1000_MDIC_READY)) {
		return SYSERR;
	}
	if (mdic & E1000_MDIC_ERROR) {
		return SYSERR;
	}

	return OK;
}
