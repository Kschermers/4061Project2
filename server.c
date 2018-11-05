/*  Class:  CSCI 4061
 *  Term:   Fall 2018
 *  Project: Project 2
 *  File: server.c
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
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "comm.h"
#include "util.h"

/* -----------Functions that implement server functionality -------------------------*/

/*
 * Returns the empty slot on success, or -1 on failure
 */
int find_empty_slot(USER * user_list) {
	// iterate through the user_list and check m_status to see if any slot is EMPTY
	// return the index of the empty slot
    int i = 0;
	for(i=0;i<MAX_USER;i++) {
    	if(user_list[i].m_status == SLOT_EMPTY) {
			return i;
		}
	}
	return -1;
}

/*
 * list the existing users on the server shell
 */
int list_users(int idx, USER * user_list)
{
	// iterate through the user list
	// if you find any slot which is not empty, print that m_user_id
	// if every slot is empty, print "<no users>""
	// If the function is called by the server (that is, idx is -1), then printf the list
	// If the function is called by the user, then send the list to the user using write() and passing m_fd_to_user
	// return 0 on success
	int i, flag = 0;
	char buf[MAX_MSG] = {}, *s = NULL;

	/* construct a list of user names */
	s = buf;
	strncpy(s, "---connecetd user list---\n", strlen("---connecetd user list---\n"));
	s += strlen("---connecetd user list---\n");
	for (i = 0; i < MAX_USER; i++) {
		if (user_list[i].m_status == SLOT_EMPTY)
			continue;
		flag = 1;
		strncpy(s, user_list[i].m_user_id, strlen(user_list[i].m_user_id));
		s = s + strlen(user_list[i].m_user_id);
		strncpy(s, "\n", 1);
		s++;
	}
	if (flag == 0) {
		strcpy(buf, "<no users>\n");
	} else {
		s--;
		strncpy(s, "\0", 1);
	}

	if(idx < 0) {
		printf("%s",buf);
		printf("\n");
	} else {
		/* write to the given pipe fd */
		if (write(user_list[idx].m_fd_to_user, buf, strlen(buf) + 1) < 0)
			perror("writing to server shell");
	}

	return 0;
}

/*
 * add a new user
 */
int add_user(int idx, USER * user_list, int pid, char * user_id, int pipe_to_child, int pipe_to_parent)
{
    // populate the user_list structure with the arguments passed to this function
    // return the index of user added
    if(idx >= 0){
        USER newUser = {};
        newUser.m_pid = pid;
        
        //how should this be declared???
        newUser.m_user_id;
        newUser.m_fd_to_user = pipe_to_child;
        newUser.m_fd_to_server = pipe_to_parent;
        newUser.m_status = SLOT_FULL;
        
        user_list[idx] = newUser;
        return idx;
    }
    else{
        return -1;
    }
}

/*
 * Kill a user
 */
void kill_user(int idx, USER * user_list) {
	// kill a user (specified by idx) by using the systemcall kill()
	// then call waitpid on the user
    kill(user_list[idx].m_pid, SIGKILL);
    int status;
    waitpid(user_list[idx].m_pid, &status,0);
}

/*
 * Perform cleanup actions after the used has been killed
 */
void cleanup_user(int idx, USER * user_list)
{
	// m_pid should be set back to -1
	// m_user_id should be set to zero, using memset()
	// close all the fd
	// set the value of all fd back to -1
	// set the status back to empty
    user_list[idx].m_pid = -1;
    memset(user_list[idx].m_user_id, '\0', MAX_USER_ID);
    close(user_list[idx].m_fd_to_user);
    close(user_list[idx].m_fd_to_server);
    user_list[idx].m_fd_to_server =  -1;
    user_list[idx].m_fd_to_user = -1;
    user_list[idx].m_status = SLOT_EMPTY;
}

/*
 * Kills the user and performs cleanup
 */
void kick_user(int idx, USER * user_list) {
    // should kill_user()
    // then perform cleanup_user()
    kill_user(idx, user_list);
    cleanup_user(idx, user_list);
}

/*
 * broadcast message to all users
 */
int broadcast_msg(USER * user_list, char *buf, char *sender)
{
	//iterate over the user_list and if a slot is full, and the user is not the sender itself,
	//then send the message to that user
	//return zero on success
	return 0;
}

/*
 * Cleanup user chat boxes
 */
void cleanup_users(USER * user_list)
{
	// go over the user list and check for any empty slots
	// call cleanup user for each of those users.
	int i;
    for(i = 0; i < MAX_USER; i++){
        if(user_list[i].m_status==SLOT_EMPTY){
            cleanup_user(i, user_list);
        }
    }
}

/*
 * find user index for given user name
 */
int find_user_index(USER * user_list, char * user_id)
{
	// go over the  user list to return the index of the user which matches the argument user_id
	// return -1 if not found
	int i, user_idx = -1;

	if (user_id == NULL) {
		fprintf(stderr, "NULL name passed.\n");
		return user_idx;
	}
	for (i=0;i<MAX_USER;i++) {
		if (user_list[i].m_status == SLOT_EMPTY)
			continue;
		if (strcmp(user_list[i].m_user_id, user_id) == 0) {
			return i;
		}
	}

	return -1;
}

/*
 * given a command's input buffer, extract name
 */
int extract_name(char * buf, char * user_name)
{
	char inbuf[MAX_MSG];
    char * tokens[16];
    strcpy(inbuf, buf);

    int token_cnt = parse_line(inbuf, tokens, " ");

    if(token_cnt >= 2) {
        strcpy(user_name, tokens[1]);
        return 0;
    }

    return -1;
}

int extract_text(char *buf, char * text)
{
    char inbuf[MAX_MSG];
    char * tokens[16];
    strcpy(inbuf, buf);

    int token_cnt = parse_line(buf, tokens, " ");

    if(token_cnt >= 3) {
        strcpy(text, tokens[2]);
        return 0;
    }

    return -1;
}

/*
 * send personal message
 */
void send_p2p_msg(int idx, USER * user_list, char *buf)
{
	// get the target user by name using extract_name() function
	// find the user id using find_user_index()
	// if user not found, write back to the original user "User not found", using the write()function on pipes. 
	// if the user is found then write the message that the user wants to send to that user.
}

//takes in the filename of the file being executed, and prints an error message stating the commands and their usage
void show_error_message(char *filename)
{
}


/*
 * Populates the user list initially
 */
void init_user_list(USER * user_list) {

	//iterate over the MAX_USER
	//memset() all m_user_id to zero
	//set all fd to -1
	//set the status to be EMPTY
	int i=0;
	for(i=0;i<MAX_USER;i++) {
		user_list[i].m_pid = -1;
		memset(user_list[i].m_user_id, '\0', MAX_USER_ID);
		user_list[i].m_fd_to_user = -1;
		user_list[i].m_fd_to_server = -1;
		user_list[i].m_status = SLOT_EMPTY;
	}
}

/* ---------------------End of the functions that implementServer functionality -----------------*/


/* ---------------------Start of the Main function ----------------------------------------------*/
int main(int argc, char * argv[])
{
	int nbytes;
	setup_connection("ok"); // Specifies the connection point as argument.

	USER user_list[MAX_USER];
	init_user_list(user_list);   // Initialize user list

	char buf[MAX_MSG]; 
	fcntl(0, F_SETFL, fcntl(0, F_GETFL)| O_NONBLOCK);
	print_prompt("admin");

    
    // arrays of pipes to handle a max of 10 children
    
    //-----Decleration of Pipes-----//
    
    //-----End of Decleratoion-----//
    
    
    
    //-----Creation of Pipes-----//
    
    //------End of Pipe Creation-----//
    
    
	/* ------------------------YOUR CODE FOR MAIN--------------------------------*/
	// declarations before loop    
	int slot;
	int pid;
    
   
    int pipe_child_from_client[2];
    int pipe_child_to_client[2];
    
    int flags, i;
    
    while(1) {
      
        
		slot = find_empty_slot(user_list);
        
        char read_child_from_client[MAX_MSG];
        char read_server_from_child[MAX_MSG];
		char user_id[MAX_USER_ID];

        if(slot>=0 && get_connection(user_id, 
					  pipe_child_to_client,
					  pipe_child_from_client)==0){
            
            //printf("DEBUG: get connection success\n\n");
            int pipe_server_from_child[2];
            int pipe_server_to_child[2];
            
            pipe(pipe_server_from_child);
            pipe(pipe_server_to_child);
            
            flags = fcntl(pipe_server_from_child[1], F_GETFL, 0);
            fcntl(pipe_server_from_child[1], F_SETFL, flags | O_NONBLOCK);
            
            flags = fcntl(pipe_server_to_child[0], F_GETFL, 0);
            fcntl(pipe_server_to_child[0], F_SETFL, flags | O_NONBLOCK);
            
            flags = fcntl(pipe_server_from_child[0], F_GETFL, 0);
            fcntl(pipe_server_from_child[0], F_SETFL, flags | O_NONBLOCK);
            
            flags = fcntl(pipe_server_to_child[1], F_GETFL, 0);
            fcntl(pipe_server_to_child[1], F_SETFL, flags | O_NONBLOCK);
            
            close(pipe_server_to_child[1]);
            close(pipe_server_from_child[0]);
            
            flags = fcntl(pipe_child_to_client[0], F_GETFL, 0);
            fcntl(pipe_child_to_client[0], F_SETFL, flags | O_NONBLOCK);
            
            flags = fcntl(pipe_child_from_client[1], F_GETFL, 0);
            fcntl(pipe_child_from_client[1], F_SETFL, flags | O_NONBLOCK);
            
            flags = fcntl(pipe_child_to_client[1], F_GETFL, 0);
            fcntl(pipe_child_to_client[1], F_SETFL, flags | O_NONBLOCK);
            
            flags = fcntl(pipe_child_from_client[0], F_GETFL, 0);
            fcntl(pipe_child_from_client[0], F_SETFL, flags | O_NONBLOCK);

            
        	pid = fork();
            if(pid == 0){
                //printf("DEBUG: inside child process\n\n");
                //close ends we don't need
                close(pipe_child_to_client[1]);
                close(pipe_child_from_client[0]);
               
                // Child process: poll users and SERVER
                //when read = 0 send message to server, pipe is broken
				printf("DEBUG: About to enter child-process loop\n\n");
                while(1){
					// POLLING USER:
			       	// read from client
					//printf("DEBUG: Attempting read - Child from Client\n\n");
                	int bytesRead = read(pipe_child_from_client[0], read_child_from_client, MAX_MSG);
        
                    if(bytesRead>0){
						//printf("DEBUG: Message read from client to child!\n\n");
                        // if something was read, send it to server
						printf("Message recieved: %s\n\n", read_child_from_client);
						
						if(write(pipe_server_from_child[1], read_child_from_client, MAX_MSG) != -1){
							//printf("DEBUG: Write success!\n\n");
						}else{
                            printf("Exited on bad write\n\n");
                            exit(-1);
                            //printf("DEBUG: Write failure!\n\n");
						}
                    }
                    
					// memset buffer
                    memset(read_child_from_client, '\0', MAX_MSG);

					// POLLING SERVER:
					// <<<<<<<NEEEDS TO BE DONE STILL>>>>>>>>
                }
                
            }else{
                // Server process: Add a new user information into an empty slot
                printf("DEBUG: Adding user\n\n");
                add_user(slot, user_list, getpid(), user_id, pipe_child_to_client[1], pipe_child_from_client[0]);
                
                int pipe_child_from_client[2];
                int pipe_child_to_client[2];
                
               
                //pipes_reading_from_client is a pipe that is assigned to m_fd_to_user
            }
        }
            
        // happens even if there isn't a new child
		// printf("DEBUG: polling children for messages...\n\n");
        for(i = 0; i < MAX_USER; i++){
        	if(user_list[i].m_status == SLOT_FULL){
				//printf("DEBUG: User slot %d is full. Attempting read...\n\n", i);
            	// poll child processes and handle user commands
				int bytesRead2 = read(user_list[i].m_fd_to_server, read_server_from_child, MAX_MSG);
				// printf("DEBUG: Read attempt complete.\n\n");
                if(bytesRead2 > 0){
                	// if something was read, write it to stdout
					//printf("DEBUG: Message read from child to server! Writing to stdout...\n\n");
					write(1, read_server_from_child, MAX_MSG);
					
                }

                //switch statement to handle commands use util function getcommandtype
            }
        }
        // Poll stdin (input from the terminal) and handle admnistrative command
        //handle commands like list, broadcast
        //make nonblocking, fix syntax
        //stdin should already be nonblocking?
        //read(0);
            
		/* ------------------------YOUR CODE FOR MAIN--------------------------------*/
	}
}

/* --------------------End of the main function ----------------------------------------*/
