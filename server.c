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
#include <signal.h>

#include "comm.h"
#include "util.h"

/* -----------Functions that implement server functionality -------------------------*/

/*
 * Returns the empty slot on success, or -1 on failure
 */
USER user_list[MAX_USER];


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
        strcpy(newUser.m_user_id, user_id);
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
    waitpid(user_list[idx].m_pid, NULL,0);
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
  int i;
  char msg[MAX_MSG];
  for(i = 0; i < MAX_USER; i++){
    if(user_list[i].m_status==SLOT_FULL){
      if(strcmp(user_list[i].m_user_id, sender) != 0){
        sprintf(msg, "%s: %s", sender, buf);
        if(write(user_list[i].m_fd_to_user, msg, MAX_MSG) == -1){
          return -1;
        }
        memset(msg, '\0', MAX_MSG);
      }
    }
  }
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

    int token_cnt = parse_line(inbuf, tokens," ");

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
    char * s = NULL;
    strcpy(inbuf, buf);

    int token_cnt = parse_line(inbuf, tokens, " ");

    if(token_cnt >= 3) {
        //Find " "
        s = strchr(buf, ' ');
        s = strchr(s+1, ' ');

        strcpy(text, s+1);
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
    char msg[MAX_MSG];
    char name[MAX_MSG];
    char txt[MAX_MSG];
      if(extract_name(buf, name)!=-1 && extract_text(buf, txt)!=-1){
          int targetIdx = find_user_index(user_list,name);
          if(targetIdx>=0){
            sprintf(msg, "%s: %s", user_list[idx].m_user_id, txt);
            write(user_list[targetIdx].m_fd_to_user, msg, MAX_MSG);
          }
          else{
            write(user_list[idx].m_fd_to_user, "User not found", 15);
          }
      }
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
int signalled = 0;

void handle_signals(int sig_num){
    int j;
    for(j = 0; j<MAX_USER; j++){
        if(user_list[j].m_status == SLOT_FULL){
            kick_user(j, user_list);
        }
    }
    signalled = 1;
}

int main(int argc, char * argv[])
{
	int nbytes;
	setup_connection("ok"); // Specifies the connection point as argument.
    
    // signal handling
    struct sigaction my_sa = {};
    my_sa.sa_handler = handle_signals;
    sigaction(SIGTERM, &my_sa, NULL);
    sigaction(SIGINT,  &my_sa, NULL);
    
	  // Initialize user list
    init_user_list(user_list);
    
	char buf[MAX_MSG];
	fcntl(0, F_SETFL, fcntl(0, F_GETFL)| O_NONBLOCK);
	print_prompt("admin");

	/* ------------------------YOUR CODE FOR MAIN--------------------------------*/
    int pipe_child_from_client[2];
    int pipe_child_to_client[2];

    int flags, i, pid;

    while(!signalled) {
        //slot where a new user could be added
        int slot = find_empty_slot(user_list);
        char buf[MAX_MSG];
        char user_id[MAX_USER_ID];
        //declare pipes
        int pipe_child_from_client[2];
        int pipe_child_to_client[2];
        //check whether there is a new user to be added, and that there is room for that user
        if(slot>=0 && get_connection(user_id, pipe_child_to_client, pipe_child_from_client)==0){
            int pipe_server_from_child[2];
            int pipe_server_to_child[2];
            pipe(pipe_server_from_child);
            pipe(pipe_server_to_child);
            //make all pipes nonblocking read and writes
            flags = fcntl(pipe_server_from_child[1], F_GETFL, 0);
            fcntl(pipe_server_from_child[1], F_SETFL, flags | O_NONBLOCK);

            flags = fcntl(pipe_server_to_child[0], F_GETFL, 0);
            fcntl(pipe_server_to_child[0], F_SETFL, flags | O_NONBLOCK);

            flags = fcntl(pipe_server_from_child[0], F_GETFL, 0);
            fcntl(pipe_server_from_child[0], F_SETFL, flags | O_NONBLOCK);

            flags = fcntl(pipe_server_to_child[1], F_GETFL, 0);
            fcntl(pipe_server_to_child[1], F_SETFL, flags | O_NONBLOCK);

            flags = fcntl(pipe_child_to_client[0], F_GETFL, 0);
            fcntl(pipe_child_to_client[0], F_SETFL, flags | O_NONBLOCK);

            flags = fcntl(pipe_child_from_client[1], F_GETFL, 0);
            fcntl(pipe_child_from_client[1], F_SETFL, flags | O_NONBLOCK);

            flags = fcntl(pipe_child_to_client[1], F_GETFL, 0);
            fcntl(pipe_child_to_client[1], F_SETFL, flags | O_NONBLOCK);

            flags = fcntl(pipe_child_from_client[0], F_GETFL, 0);
            fcntl(pipe_child_from_client[0], F_SETFL, flags | O_NONBLOCK);
            //create child process for new user
            pid = fork();
            if(pid == 0){
                //close ends child process doesn't need
                close(pipe_child_to_client[0]);
                close(pipe_child_from_client[1]);

                close(pipe_server_from_child[0]);
                close(pipe_server_to_child[1]);
                // Child process: poll users and SERVER
                while(1){
                    // POLLING USER:
                	int bytesRead = read(pipe_child_from_client[0], buf, MAX_MSG);
                    if(bytesRead==0){
                        write(pipe_server_from_child[1], "your child has died", 20);
                        exit(0);
                    }
                    if(bytesRead>0){
                        if(strcmp(buf, "your child has died") == 0){
                            write(pipe_server_from_child[1], buf, MAX_MSG);
                            memset(buf, '\0', MAX_MSG);
                            exit(0);
                        }
                        write(pipe_server_from_child[1], buf, MAX_MSG);
                        memset(buf, '\0', MAX_MSG);
                    }
                    //POLLING SERVER:
                    int bytesRead2 = read(pipe_server_to_child[0], buf, MAX_MSG);
                    if(bytesRead2 > 0){
                      write(pipe_child_to_client[1], buf, MAX_MSG);
                      memset(buf, '\0', MAX_MSG);
                    }
                    usleep(100);
                }
            }else{
                close(pipe_server_from_child[1]);
                close(pipe_server_to_child[0]);
                // Server process: Add a new user information into an empty slot
                add_user(slot, user_list, pid-1, user_id, pipe_server_to_child[1], pipe_server_from_child[0]);
            }
        }
        // poll child processes and handle user commands
        int k;
        for(i = 0; i < MAX_USER; i++){
        	if(user_list[i].m_status == SLOT_FULL){
                int bytesRead = read(user_list[i].m_fd_to_server, buf, MAX_MSG);
                if(bytesRead > 0){
                    if(strcmp(buf, "your child has died") == 0){
                        cleanup_user(i, user_list);
                        continue;
                    }
                    for(k = 0; k < MAX_MSG; k++)
                    {
                        if(buf[k]=='\n')
                        {
                            buf[k]='\0';
                            break;
                        }
                    }
                    printf("\n%s: %s\n",user_list[i].m_user_id, buf);
                    print_prompt("admin");
                    enum command_type command = get_command_type(buf);
                    if(command == P2P){
                      send_p2p_msg(i, user_list, buf);
                    }
                    else if(command == LIST){
                        list_users(i, user_list);
                    }
                    else if(command == EXIT){
                        kick_user(i, user_list);
                    }
                    else if(command == SEG){
                        write(user_list[i].m_fd_to_user, "please die, thanks", 19);
                    }
                    else{
                        broadcast_msg(user_list, buf, user_list[i].m_user_id);
                    }
                    memset(buf, '\0', MAX_MSG);
                }
            }
        }
        
        // Poll stdin (input from the terminal) and handle admnistrative command
        int bytesRead = read(0, buf, MAX_MSG);
        if(bytesRead > 0){
            for(k = 0; k < MAX_MSG; k++){
                if(buf[k]=='\n')
                {
                    buf[k]='\0';
                    break;
                }
            }
            print_prompt("admin");
            enum command_type command = get_command_type(buf);
            if(command==LIST){
                list_users(-1, user_list);
                print_prompt("admin");
            }
            else if(command == KICK){
                char name_buf[MAX_MSG];
                if(extract_name(buf, name_buf) >= 0){
                    int index = find_user_index(user_list, name_buf);
                    if(index>=0){
                        kick_user(index, user_list);
                        print_prompt("admin");
                        memset(name_buf, '\0', MAX_MSG);
                    }
                    else {
                        printf("\ncouldn't find user name: %s\n", name_buf);
                        print_prompt("admin");
                    }
                }
            }
            else if(command == EXIT){
                int j;
                for(j = 0; j<MAX_USER; j++){
                    if(user_list[j].m_status == SLOT_FULL){
                        kick_user(j, user_list);
                    }
                }
                memset(buf, '\0', MAX_MSG);
                exit(0);
            }
            else{
                broadcast_msg(user_list, buf, "admin");
            }
            memset(buf, '\0', MAX_MSG);
        }
         usleep(100);
        /* ------------------------YOUR CODE FOR MAIN--------------------------------*/
	}
}
/* --------------------End of the main function ----------------------------------------*/
