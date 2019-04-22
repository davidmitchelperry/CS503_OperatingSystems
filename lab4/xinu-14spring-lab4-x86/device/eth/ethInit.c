/* ethInit.c - ethInit */

#include <xinu.h>

struct	ether	ethertab[Neth];		/* Ethernet control blocks 	*/

/*------------------------------------------------------------------------
 * ethInit - Initialize Ethernet device structures
 *------------------------------------------------------------------------
 */
devcall	ethInit (
	  struct dentry *devptr
	)
{
	struct	ether 	*ethptr;
	int32	dinfo;			/* device information*/


	ethptr = &ethertab[devptr->dvminor];
	

	/* Initialize structure pointers */
	ethptr->dev = devptr;
	ethptr->type = NIC_TYPE_82545EM;

    /*If find the device on PCI bus, intialize the devtab of ETHER0 with the funtion pointers of e1000
     If not found, return SYSERR*/
     	ethptr->pcidev = find_pci_device(INTEL_82545EM_DEVICE_ID, INTEL_VENDOR_ID, 0);
     	if(ethptr->pcidev == SYSERR) {
		return SYSERR;
	}

	_82545EMInit(ethptr);
	
	/* Set the state of ethptr to be UP */
	ethptr->state = ETH_STATE_UP;

	return OK;
}
