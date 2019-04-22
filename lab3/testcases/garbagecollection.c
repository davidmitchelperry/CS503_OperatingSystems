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
	char *test, *test2, *test3;

	
	test = getmemgb(8);
	strncpy(test, "david", 6);

	test2 = getmemgb(8);
	strncpy(test2, "perry", 6);

	test3 = getmemgb(17);
	strncpy(test3, "mymmmmmmmmmmmmmm", 17);
	kprintf("memlist length : %d\n\r", memlist.mlength);



	return;
}

 

int main(int argc, char **argv) {
	pid32 rpid;//, rpid;

//	char *test, *test2, *test3;
//	struct memblk *curr;
//	struct procent *prptr;

	rpid = create(receiver, 2014, 20, "receiver", NULL);
	//spid = create(sender, 2048, 20, "sender", 1, rpid);

	resume(rpid);
	//sendb(rpid, 1);
	//sendb(rpid, 2);
	//sendb(rpid, 3);
	//sendb(rpid, 4);
	sleep(3);
	kprintf("memlist length after death: %d\n\r", memlist.mlength);
	//resume(spid);
	//
	//

	while(1) {
		sleep(100);
	}
	return OK;
}
