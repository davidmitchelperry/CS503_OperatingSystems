
/* The number of pipes that can exisist in the system */
#define NPIPE 10

/* Various states a pipe can be in */
#define PIPE_FREE 0
#define PIPE_USED 1
#define PIPE_CONNECTED 2
#define PIPE_NOREADER 3
#define PIPE_NOWRITER 4


/* Size of a pipe's buffer */
#define PIPE_SIZE 256

/* Definition of the pipe structure */
struct pipe {
	uint16 	state; 			/* Current state of the pipe */
	char   	buffer[PIPE_SIZE]; 	/* Pipe's buffer */
	pipid32	pipeid;			/* Unique identifier for the pipe */
	pid32 	owner; 			/* Owner of the pipe */
	pid32   reader; 		/* Reader of the pipe */
	pid32   writer; 		/* Writer of the pipe */
	sid32 	fillcount; 		/* Semaphore for filled spots in buffer */
	sid32 	emptycount; 		/* Semaphore for empty spots in buffer */
	int32 	writeindex; 		/* Index where the write should begin */
	int32 	readindex; 		/* Index where the next read should begin  */
	int32 	bytes_on_buffer; 	/* Number of bytes currently on the buffer */
};

extern struct pipe pipetab[];




