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
#include <signal.h>
#include <sys/socket.h>
#include "comm.h"
//int signalled = 0;
//void handle_signals(int sig_num){
//    printf("received signal\n");
//    signalled = 1;
//}
/* -------------------------Main function for the client ----------------------*/
void main(int argc, char * argv[]) {
	int pipe_from_child[2], pipe_to_child[2];
	int flags;

	//printf("DEBUG: connecting to server...\n\n");
	// You will need to get user name as a parameter, argv[1].
	if(connect_to_server("ok", argv[1], pipe_from_child, pipe_to_child) == -1) {
		exit(-1);
	}

    print_prompt(argv[1]);
	//printf("DEBUG: connection success!\n\n");

	/* -------------- YOUR CODE STARTS HERE -----------------------------------*/
	// signal handling
//     struct sigaction my_sa = {};
//     my_sa.sa_handler = handle_signals;
//     sigaction(SIGTERM, &my_sa, NULL);
//     sigaction(SIGINT,  &my_sa, NULL);

	// set up buffers
	char buf_send[MAX_MSG];
	char buf_recieve[MAX_MSG];
	fcntl(0, F_SETFL, fcntl(0, F_GETFL)|O_NONBLOCK);

	flags = fcntl(pipe_from_child[0], F_GETFL, 0);
	fcntl(pipe_from_child[0], F_SETFL, flags | O_NONBLOCK);

    flags = fcntl(pipe_to_child[1], F_GETFL, 0);
    fcntl(pipe_to_child[1], F_SETFL, flags | O_NONBLOCK);
    
	close(pipe_from_child[1]);
	close(pipe_to_child[0]);

    while(1){
        // poll pipe retrieved and print it to stdout
        int bytesRead = read(pipe_from_child[0], buf_recieve, MAX_MSG);
        if(bytesRead > 0){
                printf("\n%s\n", buf_recieve);
                print_prompt(argv[1]);
                memset(buf_recieve, '\0', MAX_MSG);
        }
        // Poll stdin (input from the terminal) and send it to server (child process)via pipe
        int bytesRead2 = read(0, buf_send, MAX_MSG);
        if(bytesRead2 > 0){
            print_prompt(argv[1]);
            write(pipe_to_child[1], buf_send, MAX_MSG);
            memset(buf_send, '\0', MAX_MSG);
        }
        usleep(100);
    }
    exit(0);
}
	/* -------------- YOUR CODE ENDS HERE -----------------------------------*/

/*--------------------------End of main for the client --------------------------*/
