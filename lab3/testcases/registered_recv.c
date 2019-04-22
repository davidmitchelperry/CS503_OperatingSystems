/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>

/************************************************************************/
/*									*/
/* main - main program for testing Xinu					*/
/*									*/
/************************************************************************/

pid32 reader, writer;
umsg32 recvbuf;

int myrecvhandler(void) {
	kprintf("msg received = %d\n", recvbuf);
	return(OK);
}

void reader_func(char* name, int pip) {
	kprintf("entering reader\n");
	
	registerrecv(&recvbuf, &myrecvhandler);
	while(1) {
		;
	}


}


void writer_func(char* name, int  pip) {
	kprintf("entering writer\n");
	send(reader, 10);
	send(reader, 20);
	send(reader, 30);
}

void writer_func2(char* name, int  pip) {
	sendb(reader, 3);
	sendb(reader, 4);
	sleepms(1);
}

void owner_func(char* name, int sleeptime) {
	int pip = 1;


	reader = create ((void *)reader_func, INITSTK, 25, "B", 2, "B", pip);
	writer = create ((void *)writer_func, INITSTK, 25, "C", 2, "C", pip);
	//writer2 = create ((void *)writer_func2, INITSTK, 25, "C", 2, "C", pip);

	
	//resume(writer2);
	resume(reader);
	resume(writer);
	//
	
		
}


int main(int argc, char **argv)
{
//	umsg32 retval;
//


	resume(create ((void *)owner_func, INITSTK, 25, "A", 2, "A", 1));
	while(1) {
		;
	}
		

	/* Creating a shell process */

	//resume(create(shell, 4096, 1, "shell", 1, CONSOLE));

	//retval = recvclr();
	//while (TRUE) {
	//	retval = receive();
	//	kprintf("\n\n\rMain process recreating shell\n\n\r");
	//	resume(create(shell, 4096, 1, "shell", 1, CONSOLE));
	//}

	return OK;
}
