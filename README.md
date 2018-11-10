CSci4061 F2018 Project 2
1. The purpose of your program
The purpose of our program is to implement a simple ”local” multi-party chat application using a multi-process architecture

2. A brief description of who did what on the lab
Kadin Schermers - Writing the command handlers in the SERVER program.
Sam Ball - Writing the user program.
Nicole (Nikki) Walker - Writing the server program (creating child processes).
We all helped each other with debugging and testing.

3. How to compile the program
Open 2+ terminals. In each terminal navigate to the directory with the server and client executables and makefile. In one terminal type "make" 

4. How to use the program from the shell (syntax)
Open 2+ terminals. In one terminal type ./server, in the others type ./client name where name is replaced with the name of the user.

5. What exactly your program does
Anything typed in the users will be printed out in the server, and then handled properly as either a command or a broadcast to all other users. Anything typed after the admin >> prompt will be handled as an administrative command, any text will be broadcast to all users.

6. Any explicit assumptions you have made

7. Your strategies for error handling
We implemented a signal handler in client.c that writes a message to the server indicating that it has died before dying. When that message is received in the child process it sends that message on to the server, and when it is received the proper user is cleaned up. To handle the \seg command we send a message to the user process that indicates that it should seg fault. The user process sends a message that it has died before it does so. We also implemented a signal handler in server.c that kicks and cleans up every user process before exiting itself. Finally, in the child process we check that if a read returns 0, then that pipe is broken and we should send a message to the server to clean up that process. 
