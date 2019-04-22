/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>

/************************************************************************/
/*									*/
/* main - main program for testing Xinu					*/
/*									*/
/************************************************************************/

pid32 reader, writer, writer2;

void reader_func(char* name, int pip) {

	sleep(3);
	kprintf("reader received message: %d\n", receiveb());
	kprintf("reader received message: %d\n", receiveb());
	kprintf("reader received message: %d\n", receiveb());
	kprintf("reader received message: %d\n", receiveb());
	kprintf("reader received message: %d\n", receiveb());

}


void writer_func(char* name, int  pip) {
	sendb(reader, 1);
	sendb(reader, 2);
	sleep(5);
	sendb(reader, 5);
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
	writer2 = create ((void *)writer_func2, INITSTK, 25, "C", 2, "C", pip);

	
	resume(writer);
	resume(writer2);
	resume(reader);
		
}

int main(int argc, char **argv)
{
//	umsg32 retval;

	resume(create ((void *)owner_func, INITSTK, 20, "A", 2, "A", 1));
		

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
