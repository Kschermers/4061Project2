/*  Class:  CSCI 4061
 *  Term:   Fall 2018
 *  Project: Project 2
 *  File: client.c
 *
 *  Written by:
 *      Nikki Walker: walk0760@umn.edu
 *      Kadin Schermers: scher528@umn.edu
 *      Samuel Ball: ballx188@umn.edu
 *
 *  This file contains code solely supplied by our superiors for our use
 *  or code that has been written by us.
 *
 *  Members of Project 2 Group 17 on Canvas
 *  University of Minnesota Fall 2018
 */


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
int signalled = 0;
void handle_signals(int sig_num){signalled = 1;}
/* -------------------------Main function for the client ----------------------*/
void main(int argc, char * argv[]) {
	int pipe_to_user[2], pipe_to_server[2];
	int flags;

	printf("DEBUG: connecting to server...\n\n");
	// You will need to get user name as a parameter, argv[1].
	if(connect_to_server("ok", argv[1], pipe_to_user, pipe_to_server) == -1) {
		exit(-1);
	}
	printf("DEBUG: connection success!\n\n");

	/* -------------- YOUR CODE STARTS HERE -----------------------------------*/
	// signal handling
    struct sigaction my_sa = {};
    my_sa.sa_handler = handle_signals;
    sigaction(SIGTERM, &my_sa, NULL);
    sigaction(SIGINT,  &my_sa, NULL);
    
	// set up buffers
	char buf_send[MAX_MSG];
	char buf_recieve[MAX_MSG];	

	// close unused pipe ends
    //Should we close these here?? Cause of the comm methods creating the pipe thing..
	close(pipe_to_user[1]);
	close(pipe_to_server[0]);
	printf("DEBUG: Closed pipes!\n\n");
	
	// make reading from server nonblocking
	flags = fcntl(pipe_to_user[0], F_GETFL, 0);
	fcntl(pipe_to_user[0], F_SETFL, flags | O_NONBLOCK);
	printf("DEBUG: Read from server set to nonblocking!\n\n");

	// make stdin nonblocking
    //flags = fcntl(0, F_GETFL, 0); /* get current file status flags */
    //flags |= O_NONBLOCK;    /* turn off blocking flag */
    //fcntl(0, F_SETFL,0);    /* set up non-blocking read */

	printf("DEBUG: About to enter user-process loop\n\n");
	while(!signalled){
		// poll pipe retrieved and print it to stdout
		printf("DEBUG: polling pipe\n\n");
		int bytesRead = read(pipe_to_user[0], buf_recieve, MAX_MSG);
		printf("DEBUG: Read from pipe complete!\n\n");
        if(bytesRead > 0){
			printf("DEBUG: >0 bytes read from pipe\n\n");
            if(write(1, buf_recieve, MAX_MSG) != -1){
				printf("DEBUG: Write of message to stdout succeeded!\n\n");
			}else{
				printf("DEBUG: Write of message to stdout failed\n\n");
			}
        }
		memset(buf_recieve, '\0', MAX_MSG);

		// Poll stdin (input from the terminal) and send it to server (child process) via pipe
      	printf("DEBUG: polling stdin...\n\n");
		int bytesRead2 = read(0, buf_send, MAX_MSG);
		printf("DEBUG: Read from stdin complete\n\n");
        if(bytesRead2 > 0){
			printf("DEBUG: >0 bytes read from stdin\n\n");
            if(write(pipe_to_server[1], buf_send, MAX_MSG) != -1){
				printf("DEBUG: Write from client to server succeeded!\n\n");
			}else{
				printf("DEBUG: Write from client to server failed\n\n");
			}
        }
		memset(buf_send, '\0', MAX_MSG);
		printf("DEBUG: End of user-process loop\n\n");
	}	
	/* -------------- YOUR CODE ENDS HERE -----------------------------------*/
}

/*--------------------------End of main for the client --------------------------*/


