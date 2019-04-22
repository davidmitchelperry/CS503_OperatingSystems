/* xsh_gen.c - xsh_gen */

#include <xinu.h>
#include <stdio.h>
#include "wordlist.h"
#include <string.h>
#include <stdlib.h>

/*------------------------------------------------------------------------
 * xhs_gen 
 * *------------------------------------------------------------------------
 */
shellcmd xsh_gen(int nargs, char *args[])
{

	pipid32 pipe;
	uint32 timestamp;
	int ct = 0;
	int randval;
	char *word;

	//Error checking arguments
	if(nargs != 2) {
		kprintf("invalid use of program\n");
		return 0;
	}

	//Get pipe id
	pipe = atoi(args[1]);

	//seed the rng
	srand(1);
	//set the timestamp
	timestamp = clktime;
	while(1) {
		//get a random number	
		randval = rand() % 2048;
		//get a randomw word
		word = words[randval];

		//perform the pipe write
		if(pipwrite(pipe, word, 4) == SYSERR) {
			kprintf("Pipe Writer Error");
			break;
		}
		else {
			//Check if 5 secs have past
			if(clktime - timestamp >= 5) {
				//reset timestamp
				timestamp = clktime;
				kprintf("words written: %d", ct);
			}
		}
		ct++;
	}

	return 0;
}
