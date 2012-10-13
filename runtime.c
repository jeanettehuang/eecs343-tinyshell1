/***************************************************************************
 *  Title: Runtime environment 
 * -------------------------------------------------------------------------
 *    Purpose: Runs commands
 *    Author: Stefan Birrer
 *    Version: $Revision: 1.3 $
 *    Last Modification: $Date: 2009/10/12 20:50:12 $
 *    File: $RCSfile: runtime.c,v $
 *    Copyright: (C) 2002 by Stefan Birrer
 ***************************************************************************/
/***************************************************************************
 *  ChangeLog:
 * -------------------------------------------------------------------------
 *    $Log: runtime.c,v $
 *    Revision 1.3  2009/10/12 20:50:12  jot836
 *    Commented tsh C files
 *
 *    Revision 1.2  2009/10/11 04:45:50  npb853
 *    Changing the identation of the project to be GNU.
 *
 *    Revision 1.1  2005/10/13 05:24:59  sbirrer
 *    - added the skeleton files
 *
 *    Revision 1.6  2002/10/24 21:32:47  sempi
 *    final release
 *
 *    Revision 1.5  2002/10/23 21:54:27  sempi
 *    beta release
 *
 *    Revision 1.4  2002/10/21 04:49:35  sempi
 *    minor correction
 *
 *    Revision 1.3  2002/10/21 04:47:05  sempi
 *    Milestone 2 beta
 *
 *    Revision 1.2  2002/10/15 20:37:26  sempi
 *    Comments updated
 *
 *    Revision 1.1  2002/10/15 20:20:56  sempi
 *    Milestone 1
 *
 ***************************************************************************/
#define __RUNTIME_IMPL__

/************System include***********************************************/
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

/************Private include**********************************************/
#include "runtime.h"
#include "io.h"

/************Defines and Typedefs*****************************************/
/*  #defines and typedefs should have their names in all caps.
 *  Global variables begin with g. Global constants with k. Local
 *  variables should be in all lower case. When initializing
 *  structures and arrays, line everything up in neat columns.
 */

/************Global Variables*********************************************/

#define NBUILTINCOMMANDS (sizeof BuiltInCmds / sizeof(char*))

char* BuiltInCmds[3] = {"pwd","exit","cd"};

typedef struct bgjob_l
{
  pid_t pid;
  struct bgjob_l* next;
} bgjobL;

/* the pids of the background processes */
bgjobL *bgjobs = NULL;

/* fg child pid */
pid_t fgChildPid = 0;

/************Function Prototypes******************************************/
/* run command */
static void
RunCmdFork(commandT*, bool);
/* runs an external program command after some checks */
static void
RunExternalCmd(commandT*, bool);
/* resolves the path and checks for exutable flag */
static bool
ResolveExternalCmd(commandT*);
/* forks and runs a external program */
static void
Exec(char*, commandT*, bool);
/* runs a builtin command */
static void
RunBuiltInCmd(commandT*);
/* checks whether a command is a builtin command */
static bool
IsBuiltIn(char*);
/* checks whether a file exists and is executable */
static bool
existsAndExecutable(char*);
/* returns the complete file path for a command */
char*
getCompleteFilePath(char*);
/************External Declaration*****************************************/

/**************Implementation***********************************************/


/*
 * RunCmd
 *
 * arguments:
 *   commandT *cmd: the command to be run
 *
 * returns: none
 *
 * Runs the given command.
 */
void
RunCmd(commandT* cmd)
{
  RunCmdFork(cmd, TRUE);
} /* RunCmd */


/*
 * RunCmdFork
 *
 * arguments:
 *   commandT *cmd: the command to be run
 *   bool fork: whether to fork
 *
 * returns: none
 *
 * Runs a command, switching between built-in and external mode
 * depending on cmd->argv[0].
 */
void
RunCmdFork(commandT* cmd, bool fork)
{
  if (cmd->argc <= 0)
    return;
  if (IsBuiltIn(cmd->argv[0]))
    {
      RunBuiltInCmd(cmd);
    }
  else
    {
      RunExternalCmd(cmd, fork);
    }
} /* RunCmdFork */


/*
 * RunCmdBg
 *
 * arguments:
 *   commandT *cmd: the command to be run
 *
 * returns: none
 *
 * Runs a command in the background.
 */
void
RunCmdBg(commandT* cmd)
{
  // TODO
} /* RunCmdBg */


/*
 * RunCmdPipe
 *
 * arguments:
 *   commandT *cmd1: the commandT struct for the left hand side of the pipe
 *   commandT *cmd2: the commandT struct for the right hand side of the pipe
 *
 * returns: none
 *
 * Runs two commands, redirecting standard output from the first to
 * standard input on the second.
 */
void
RunCmdPipe(commandT* cmd1, commandT* cmd2)
{
} /* RunCmdPipe */


/*
 * RunCmdRedirOut
 *
 * arguments:
 *   commandT *cmd: the command to be run
 *   char *file: the file to be used for standard output
 *
 * returns: none
 *
 * Runs a command, redirecting standard output to a file.
 */
void
RunCmdRedirOut(commandT* cmd, char* file)
{
} /* RunCmdRedirOut */


/*
 * RunCmdRedirIn
 *
 * arguments:
 *   commandT *cmd: the command to be run
 *   char *file: the file to be used for standard input
 *
 * returns: none
 *
 * Runs a command, redirecting a file to standard input.
 */
void
RunCmdRedirIn(commandT* cmd, char* file)
{
}  /* RunCmdRedirIn */


/*
 * RunExternalCmd
 *
 * arguments:
 *   commandT *cmd: the command to be run
 *   bool fork: whether to fork
 *
 * returns: none
 *
 * Tries to run an external command.
 */
static void
RunExternalCmd(commandT* cmd, bool fork){
  if (ResolveExternalCmd(cmd)) {
    Exec(cmd->name,cmd,fork);
  }
}  /* RunExternalCmd */


/*
 * ResolveExternalCmd
 *
 * arguments:
 *   commandT *cmd: the command to be run
 *
 * returns: bool: whether the given command exists
 *
 * Determines whether the command to be run actually exists.
 */
static bool
ResolveExternalCmd(commandT* cmd) {
  char* completeFilePath = getCompleteFilePath(cmd->name);

  if (completeFilePath != NULL) {
    cmd->name = completeFilePath;
    return TRUE;
  }

  free(completeFilePath);
  return FALSE;
} /* ResolveExternalCmd */


/*
 * Exec
 *
 * arguments:
 *   commandT *cmd: the command to be run
 *   bool forceFork: whether to fork
 *
 * returns: none
 *
 * Executes a command.
 */
static void
Exec(char* path, commandT* cmd, bool forceFork) {
  if(forceFork) {
    int status;

    // Create child process
    pid_t pid = fork();

    if (pid < 0) {
      Print("Error: Could not create new process");
    }
    // Child process executes this code
    else if (pid == 0) {
      setpgid(0, 0);
      execv(path,cmd->argv);
      fflush(stdout);
      _exit(0);
    }
    // Parent process executes this code
    else {
    do {
      waitpid(pid, &status, 0);
    }
    while(!WIFEXITED(status));
      fflush(stdout);
    }
  }
  else {
    execv(path,cmd->argv);
  }
} /* Exec */



/*
 * IsBuiltIn
 *
 * arguments:
 *   char *cmd: a command string (e.g. the first token of the command line)
 *
 * returns: bool: TRUE if the command string corresponds to a built-in
 *                command, else FALSE.
 *
 * Checks whether the given string corresponds to a supported built-in
 * command.
 */
static bool
IsBuiltIn(char* cmd) {
  int i;
  for (i = 0; i < NBUILTINCOMMANDS; i++) {
    if (strcmp(cmd, BuiltInCmds[i]) == 0) {
      return TRUE;
    }
  }
  return FALSE;
} /* IsBuiltIn */


/*
 * RunBuiltInCmd
 *
 * arguments:
 *   commandT *cmd: the command to be run
 *
 * returns: none
 *
 * Runs a built-in command.
 */
static void
RunBuiltInCmd(commandT* cmd) {
  // Implementation of exit
  if (strcmp(cmd->argv[0], "exit") == 0) {
    return;
  }

  // Implementation of pwd
  if (strcmp(cmd->argv[0], "pwd") == 0) {
    char cwd[256];
    getcwd(cwd, 256);
    Print(cwd);
    return;
  }

  // Implementation of cd 
  if (strcmp(cmd->argv[0], "cd") == 0) {
    if (cmd->argc == 1) { // No args given, chdir to home
      chdir(getenv("HOME"));
    }
    else if (cmd->argc == 2) {
      chdir(cmd->argv[1]);
    }
    else { // More than two args given, error
      Print("Error: cd failure");
    }
  }
} /* RunBuiltInCmd */



/*
 * getCurrentWorkingDir
 *
 * arguments: none
 *
 * returns: char*: a pointer to a string that contains the current working dir
 *
 * Gets the current working directory
 */
 char*
 getCurrentWorkingDir() {
  char* currentWorkingDir = malloc(sizeof(char*)*PATH_MAX);

  if (getcwd(currentWorkingDir, PATH_MAX) == NULL) {
    free(currentWorkingDir);
    return NULL;
  }

  return currentWorkingDir;
 } /* getCurrentWorkingDir*/

/* 
 * existsAndExecutable
 *
 * arguments: char* file_path: the file path of the  file to check
 *
 * returns: bool: whether or not the file exists and is executable
 *
 * Checks the file to see if it exists and is executable
 */
 static bool
 existsAndExecutable(char* file_path) {
  struct stat file;
  if (stat(file_path, &file) == 0) {
    if ((access(file_path, X_OK) == 0) && S_ISREG(file.st_mode)) {
      return TRUE;
    }
  }
  return FALSE;
 } /*existsandExecutable*/

/*
 * getCompleteFilePath
 *
 * arguments: char* file: the file whose full path we want to find
 *
 */
char*
getCompleteFilePath(char* file) {

  bool inPath = FALSE;
  char* paths = getenv("PATH");
  char* res = malloc(sizeof(char*)*PATH_MAX);

  // Path is an absolute path, do not need to manip path to look up file
  if (file[0] == '/') {
    if (existsAndExecutable(file)) {
       strcpy(res, file);
       inPath = TRUE;
    }
  } 
  else {
  // Check if file exists in our homeDir dir
    char* homeDirPath = malloc(sizeof(char*)*PATH_MAX);
    char* homeDir = getenv("HOME");
    strcpy(homeDirPath, homeDir);
    strcat(homeDirPath, "/");
    strcat(homeDirPath, file);
    if (existsAndExecutable(homeDirPath)) {
      strcpy(result, homeDirPath);
      inPath = TRUE;
    } else {
      // Check if file exists in our current dir
      char* cwd = getCurrentWorkingDir();
      char* cwdWithFilename = malloc(sizeof(char*)*PATH_MAX);
      strcpy(cwdWithFilename, cwd);
      strcat(cwdWithFilename, "/");
      strcat(cwdWithFilename, file);
      if (existsAndExecutable(cwdWithFilename)) {
        strcpy(result, cwdWithFilename);
        inPath = TRUE;
      } 
      else {
      // Otherwise see if it exists in any of the folders in our path.
        char* newPath = malloc(sizeof(char*)*PATH_MAX);
        strcpy(newPath, paths);
        char* path = strtok(newPath, ",");
        while (path != NULL) {
          char* pathWithFilename = malloc(sizeof(char*)*PATH_MAX);
          strcpy(pathWithFilename, path);
          strcat(pathWithFilename, "/");
          strcat(pathWithFilename, file);
          if (existsAndExecutable(pathWithFilename)) {
            strcpy(result, pathWithFilename);
            inPath = TRUE;
          }
          path = strtok(NULL, ",");
          free(pathWithFilename);
        }
        free(newPath);
      }
      free(cwdWithFilename);
      free(cwd);
    }
    free(homeDirPath);
  }

  if (inPath) {
    return result;
  } 
  else {
    strcpy(result, "line 1: ");
    strcat(result, file);
    PrintPError(result);
    free(result);
    return NULL;
  }

} /* getCompleteFilePath */



/*
 * StopFgProc
 *
 * arguments: none
 *
 * returns: none
 *
 * If there is a fg process group, StopFgProc will stop it
 */
 void
 StopFgProc() {
  if (fgChildPid != 0) {
    kill((fgChildPid * -1), SIGINT);
  }
 }



/*
 * CheckJobs
 *
 * arguments: none
 *
 * returns: none
 *
 * Checks the status of running jobs.
 */
void
CheckJobs()
{
} /* CheckJobs */
