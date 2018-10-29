#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include "comm.h"

/* -------------------------Main function for the client ----------------------*/
void main(int argc, char * argv[]) {

	int pipe_to_user[2], pipe_to_server[2];
	pipe(pipe_to_user);
	pipe(pipe_to_server);

	// You will need to get user name as a parameter, argv[1].

	if(connect_to_server("ok", argv[1], pipe_to_user, pipe_to_server) == -1) {
		exit(-1);
	}

	/* -------------- YOUR CODE STARTS HERE -----------------------------------*/
	char buffer[MAX_MSG];
	
	close(pipe_to_user[1]);
	close(pipe_to_server[0]);
    int flags = fcntl(0, F_GETFL, 0); /* get current file status flags */
    flags |= O_NONBLOCK;    /* turn off blocking flag */
    fcntl(0, F_SETFL, val);    /* set up non-blocking read */
	while(){
		// poll pipe retrieved and print it to sdiout
		//read(pipe_to_user[0], , MAX_MSG)



		// Poll stdin (input from the terminal) and send it to server (child process) via pipe
      
		read(0, buffer, MAX_MSG);
		write(pipe_to_server[1], buffer, MAX_MSG);
		memset(buffer, '\0', MAX_MSG);

	}	
	/* -------------- YOUR CODE ENDS HERE -----------------------------------*/
}

/*--------------------------End of main for the client --------------------------*/


