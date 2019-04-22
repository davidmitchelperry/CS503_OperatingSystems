3/5/2014

Implemented Pipes in xinu. The following files have been updated
or added:

pipcreate.c:
	initializes a pipe and returns the pipe id

pipconnect.c:
	Assigns two different processes to be the reader or 
	writer of the pipe.

pipdisconnect.c:
	Removes the reader and writer from the pipe. The 
	pipe can still be used again if reconnected to.

pipdelete.c:
	Deletes the pipe and makes it available for other
	processes to use.

pipwrite.c:
	Allows the writer process to write data to the pipe

pipread.c:
	Allows the reader process to read data from the pipe

kill.c
	Modified kill function to clean up pipes appropriately
	when the owner, reader, or writer are killed.

Restrictions:

	In this implementation a proces can only be the owner or
	writer of 1 pipe. This makes implementing the kill function
	significantly easier.

gen | search:

	Implemented the gen | search command to the xinu shell to behave
	according to the spec. | is recognized as a token. Gen and search
	will not work without the pipe or when called without the other
	process.

	Restriction:
		Gen is designed to pull words from wordlist.h. Therefore,
		gen only writes a hardcoded value of 4. Since all words
		in wordlist.h are of length 4 this is no problem.
