/* xsh_search.c - xsh_search */

#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>

/*------------------------------------------------------------------------
 * xhs_search 
 *------------------------------------------------------------------------
 */
shellcmd xsh_search(int nargs, char *args[])
{
	
	pipid32 pipe;
	char buf[4];
	uint32 timestamp;

	int a = 0;
	int e = 0;
	int i = 0;
	int o = 0;
	int u = 0;
	
	//Error checking arguments
	if(nargs != 2) {
		kprintf("invalid use of program\n");
		return 0;
	}
	//get pipe id
	pipe = atoi(args[1]);
	//set time stamp
	timestamp = clktime;
	while(1) {
		//increment the correct counter
		if(pipread(pipe, buf, 4) != SYSERR) {
			if(buf[0] == 'A') {
				a++;
			}
			if(buf[0] == 'E') {
				e++;
			}
			if(buf[0] == 'I') {
				i++;
			}
			if(buf[0] == 'O') {
				o++;
			}
			if(buf[0] == 'U') {
				u++;
			}
			
		}
		else {
			kprintf("reader error");
			break;
		}
		//check if 10 seconds have past
		if(clktime - timestamp >= 10) {
			//reset timestamp
			timestamp = clktime;
			kprintf("a: %d, e: %d, i: %d, o: %d, u: %d\n", a, e, i, o, u);
		}
	}


	return 0;
}
