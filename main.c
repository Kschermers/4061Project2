/* CSci4061 F2018 Assignment 1
* login: cselabs login name (login used to submit)
* date: mm/dd/yy
* name: full name1, full name2, full name3 (for partner(s))
* id: id for first name, id for second name, id for third name */

// This is the main file for the code
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "util.h"

/*-------------------------------------------------------HELPER FUNCTIONS PROTOTYPES---------------------------------*/
void show_error_message(char * ExecName);
//Write your functions prototypes here
void show_targets(target_t targets[], int nTargetCount);
/*-------------------------------------------------------END OF HELPER FUNCTIONS PROTOTYPES--------------------------*/


/*-------------------------------------------------------HELPER FUNCTIONS--------------------------------------------*/

//This is the function for writing an error to the stream
//It prints the same message in all the cases
void show_error_message(char * ExecName)
{
  fprintf(stderr, "Usage: %s [options] [target] : only single target is allowed.\n", ExecName);
  fprintf(stderr, "-f FILE\t\tRead FILE as a makefile.\n");
  fprintf(stderr, "-h\t\tPrint this message and exit.\n");
  exit(0);
}

//Write your functions here

// This function sets all target Status fields to 0.
// Based on our implementation Status is set to 0 or 1.
// 0 indicates that the target has not been built.
// 1 indicates that the target has been built already.
void set_status_to_zero(target_t targets[], int nTargetCount)
{
  for (int i=0; i < nTargetCount; i++) {
    targets[i].Status = 0;
  }
}

// This function uses execvp to execute a shell commmand.
void execute_command(char * targetCommand)
{
  // Create array of strings for command arguments.
  char *commandArgs[10];
  // Parse command into tokens.
  int numberTokens = parse_into_tokens(targetCommand, commandArgs, " ");
  // Execute command.
  execvp(commandArgs[0], commandArgs);
  // Return if exec doesn't work. build_from_target will handle the error.
}

// This function returns 1 if the dependencies are newer than the target.
// If 1, the target command needs to be executed.
int is_target_outdated(target_t target)
{
  char *targetName = target.TargetName;
  int dependencyCount = target.DependencyCount;

  // Check all dependencies.
  for (int i=0; i<dependencyCount; i++) {
    char *dependencyName = target.DependencyNames[i];
    if (compare_modification_time(targetName, dependencyName) == 2 ||
        compare_modification_time(targetName, dependencyName) == -1)
    {
      // Return true is target is older than a dependency.
      return 1;
    }
  }
  // Return false if target is up to date.
  return 0;
}


// This function will recursively build the executable files based on the initial target name.
void build_from_target(char * targetName, target_t targets[], int nTargetCount)
{
  // Get target node number using target name.
  int targetNodeNum = find_target(targetName, targets, nTargetCount);

  // Check if targetNodeNum is postive, else target doesn't exist.
  if (targetNodeNum >= 0) {
    // Get target structure from target name.
    target_t currentTarget = targets[targetNodeNum];
    printf("target name %s status %d\n", currentTarget.TargetName, currentTarget.Status);

    // Check if target has been built.
    if (currentTarget.Status == 0)
    {
      // Set Status to 1 so the target will not be built again.
      currentTarget.Status = 1;
      printf("target name %s status %d\n", currentTarget.TargetName, currentTarget.Status);

      // Take care of dependencies first
      if (currentTarget.DependencyCount > 0) {
        for (int i=0; i<currentTarget.DependencyCount; i++) {
          // Get dependency name from target dependencies array
          char *childTargetName = currentTarget.DependencyNames[i];

          // Fork
          // Parent executes current target.
          // Child executes dependencies.
          int pid;
          pid = fork();
          // Child
          if (pid == 0) {
            // Recursively take care of child.
            build_from_target(childTargetName, targets, nTargetCount);
          }
          // Parent
          else if (pid > 0) {
            int wstatus;
            wait(&wstatus);
            if (WEXITSTATUS(wstatus) != 0) {
              printf("Child %s exited from parent %s with error code=%d\n%s\n", currentTarget.TargetName, childTargetName, WEXITSTATUS(wstatus),strerror(errno));
              exit(-1);
            }
          }
          // Error
          else {
            // If pid < 0, fork failed.
            exit(-1);
          }
        }
      } // Dependencies are now taken care of. Target can execute command if needed.

      // Dependencies are resolved, move to command execution.
      // Check if target is outdated or doesn't have dependencies.
      // If so, execute command.
      if (is_target_outdated(currentTarget) || currentTarget.DependencyCount == 0) {
        printf("%s\n", currentTarget.Command);
        execute_command(currentTarget.Command);
        // If exec doesn't work, print error message and exit.
        exit(-1);
      }
      // Else, exit process.
      else {
        exit(0);
      }
    }
  }
  // Exit process for non-target dependency.
  else {
    exit(0);
  }
}

//Phase1: Warmup phase for parsing the structure here. Do it as per the PDF (Writeup)
void show_targets(target_t targets[], int nTargetCount)
{
  //Write your warmup code here
  printf("Printing target info:\n\n");
  for (int i=0; i < nTargetCount; i++) {
    printf("Target Name = %s\n", targets[i].TargetName);
    printf("Number of Dependencies = %d\n", targets[i].DependencyCount);
    printf("Dependency Names =");
    for (int j=0; j < targets[i].DependencyCount; j++) {
      printf(" %s", targets[i].DependencyNames[j]);
    }
    printf("\n");
    printf("Target Command = %s\n\n", targets[i].Command);
  }
}

/*-------------------------------------------------------END OF HELPER FUNCTIONS-------------------------------------*/


/*-------------------------------------------------------MAIN PROGRAM------------------------------------------------*/
//Main commencement
int main(int argc, char *argv[])
{
  target_t targets[MAX_NODES];
  int nTargetCount = 0;

  /* Variables you'll want to use */
  char Makefile[64] = "Makefile";
  char TargetName[64];

  /* Declarations for getopt. For better understanding of the function use the man command i.e. "man getopt" */
  extern int optind;      // It is the index of the next element of the argv[] that is going to be processed
  extern char * optarg;   // It points to the option argument
  int ch;
  char *format = "f:h";
  char *temp;

  //Getopt function is used to access the command line arguments. However there can be arguments which may or may not need the parameters after the command
  //Example -f <filename> needs a finename, and therefore we need to give a colon after that sort of argument
  //Ex. f: for h there won't be any argument hence we are not going to do the same for h, hence "f:h"
  while((ch = getopt(argc, argv, format)) != -1)
  {
    switch(ch)
    {
        case 'f':
          temp = strdup(optarg);
          strcpy(Makefile, temp);  // here the strdup returns a string and that is later copied using the strcpy
          free(temp); //need to manually free the pointer
          break;

        case 'h':
        default:
          show_error_message(argv[0]);
          exit(1);
    }

  }

  argc -= optind;
  if(argc > 1)   //Means that we are giving more than 1 target which is not accepted
  {
    show_error_message(argv[0]);
    return -1;   //This line is not needed
  }

  /* Init Targets */
  memset(targets, 0, sizeof(targets));   //initialize all the nodes first, just to avoid the valgrind checks

  /* Parse graph file or die, This is the main function to perform the toplogical sort and hence populate the structure */
  if((nTargetCount = parse(Makefile, targets)) == -1)  //here the parser returns the starting address of the array of the structure. Here we gave the makefile and then it just does the parsing of the makefile and then it has created array of the nodes
    return -1;


  //Phase1: Warmup-----------------------------------------------------------------------------------------------------
  //Parse the structure elements and print them as mentioned in the Project Writeup
  /* Comment out the following line before Phase2 */
  //show_targets(targets, nTargetCount);  
  //End of Warmup------------------------------------------------------------------------------------------------------
   
  /*
   * Set Targetname
   * If target is not set, set it to default (first target from makefile)
   */
  if(argc == 1)
    strcpy(TargetName, argv[optind]);    // here we have the given target, this acts as a method to begin the building
  else
    strcpy(TargetName, targets[0].TargetName);  // default part is the first target

  /*
   * Now, the file has been parsed and the targets have been named.
   * You'll now want to check all dependencies (whether they are
   * available targets or files) and then execute the target that
   * was specified on the command line, along with their dependencies,
   * etc. Else if no target is mentioned then build the first target
   * found in Makefile.
   */

  //Phase2: Begins ----------------------------------------------------------------------------------------------------
  /*Your code begins here*/

  // Set all target Status fields to 0.
  // Based on our implementation Status is set to 0 or 1.
  // 0 indicates that the target has not been built.
  // 1 indicates that the target has been built already.
  set_status_to_zero(targets, nTargetCount);

  // This function will work its way through the tree from the TargetName and execute all commands as appropriate.
  build_from_target(TargetName, targets, nTargetCount);
  
  /*End of your code*/
  //End of Phase2------------------------------------------------------------------------------------------------------

  return 0;
}
/*-------------------------------------------------------END OF MAIN PROGRAM------------------------------------------*/
