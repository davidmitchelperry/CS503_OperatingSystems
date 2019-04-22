#include <xinu.h>
#include <stdio.h>
#include <string.h>

uint32 recvbuf;

int myrecvhandler(void) {
	kprintf("msg received = %d\n", recvbuf);
	return(OK);
}

 

void sender(pid32 receiver) {
	sleep(3);
	if( send(receiver, 20) == SYSERR ) {
		kprintf("Fail to send msg 20!\r\n");
	} else {
		kprintf("Send msg 20 to receiver!\r\n");
	}
	return;
}
void receiver(void) {
	if (registerrecv(&recvbuf, &myrecvhandler) != OK) {
		kprintf("recv handler registration failed\n");
		return;
	}

	while(1) {
		sleep(1);
	}
	return;
}

 

int main(int argc, char **argv) {
	//pid32 spid, rpid;

	char *test, *test2, *test3;
	struct memblk *curr;
	struct procent *prptr;

	//rpid = create(receiver, 2014, 20, "receiver", NULL);
	//spid = create(sender, 2048, 20, "sender", 1, rpid);

	//resume(rpid);
	//resume(spid);
	//
	//
	test = getmemgb(8);
	strncpy(test, "david", 6);

	test2 = getmemgb(8);
	strncpy(test2, "perry", 6);

	test3 = getmemgb(17);
	strncpy(test3, "mymmmmmmmmmmmmmm", 17);

	kprintf("dynamic string test says: %s\n\r", test);
	kprintf("dynamic string test's address: %08X\n\r", test);
	kprintf("dynamic string test says: %s\n\r", test2);
	kprintf("dynamic string test's address: %08X\n\r", test2);
	kprintf("dynamic string test says: %s\n\r", test3);
	kprintf("dynamic string test's address: %08X\n\r", test3);

	prptr = &proctab[currpid];
	curr = prptr->mem_reference_head;
	//kprintf("nbytes :%d", curr->mlength);
	while(curr != NULL) {
		kprintf("reference blocks address: %08X\n\r", curr);
		kprintf("reference blocks size: %d\n\r", curr->mlength);
		curr = curr->mnext;
	}
		
	//while(curr->mnext != NULL) {
	//	kprintf("in\n");
	//	kprintf("reference blocks address: %08X\n", curr->heap_address);
	//	kprintf("reference blocks size: %d\n", curr->mlength);
	//	curr = curr->mnext;
	//}
	//

	kprintf("memlist length : %d\n\r", memlist.mlength);
	freememgb(test, 8);
	kprintf("memlist length after 1 free: %d\n\r", memlist.mlength);
	//freememgb(test2, 8);
	//kprintf("memlist length after 2 frees: %d\n\r", memlist.mlength);
	curr = prptr->mem_reference_head;
	//kprintf("nbytes :%d", curr->mlength);
	while(curr != NULL) {
		kprintf("reference blocks address: %08X\n\r", curr);
		kprintf("reference blocks size: %d\n\r", curr->mlength);
		curr = curr->mnext;
	}


	while(1) {
		sleep(100);
	}
	return OK;
}
