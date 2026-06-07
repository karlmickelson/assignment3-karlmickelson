#include "systemcalls.h"
#include <stdarg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define _XOPEN_SOURCE
#define _GNU_SOURCE 
#include <stdlib.h>
#include <syslog.h>  
#include <sys/stat.h> 
#include <stdbool.h> 
#include <fcntl.h> 
#include <stdio.h> 
#include <string.h> 
#include <stdarg.h> 
#include <sys/stat.h>
#include <errno.h> 
pid_t wait (int *status);
void exit (int status);
/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{
/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
    int cmdrtn;
    int system (const char *command);
    //system("echo Karl: do_system, here is a system call:");
    cmdrtn = system (cmd);
    syslog(LOG_USER, "Assignment 3-0, do_system: cmdrtn = %d\n", cmdrtn);
    syslog(LOG_USER, "Assignment 3-1, do_system: command = %s\n",cmd);
    if (WIFSIGNALED (cmdrtn) && 
         (WTERMSIG (cmdrtn) == SIGINT ||
          WTERMSIG (cmdrtn) == SIGQUIT))
             return false;  
    //if (cmdrtn == -1) { return false; }
    if (cmdrtn != 0) { return false; }
    return true;
}
/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/
bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char *command[count+1];
    int i;
    pid_t pid;
    int wait_status;
    
    for(i=0; i<count; i++)
      {
        command[i] = va_arg(args, char *);
      }
    command[count] = NULL;
    va_end(args);
    errno = 0;
    pid = fork ();
    if (pid == -1)
        exit(EXIT_FAILURE);  //fork failed
    else if (pid == 0) 
      {  //child process
        syslog(LOG_USER, "Assignment 3-3,  do_exec:     child,  pid = %d, path = %s",getpid(),command[0]);
        for(i=0; i<count+1; i++) {
          syslog(LOG_USER, "Assignment 3-4,  do_exec:     child,  pid = %d, command[%d] = %s",getpid(),i,command[i]);  }    
        errno = 0;
        
        if (execv(command[0], command) == -1)   {
            exit(EXIT_FAILURE);  } //exec failed
        exit(EXIT_FAILURE);
         
        syslog(LOG_USER, "Assignment 3-5,  do_exec:     child,  pid = %d, errno = %s",getpid(),strerror(errno));
        exit(EXIT_FAILURE);
      }
    else    
      {  //parent process   
        waitpid (-1, &wait_status, 0);
        syslog(LOG_USER, "Assignment 3-10, do_exec:     parent, pid = %d, WIFEXITED (wait_status) = %d, WEXITSTATUS (wait_status) = %d",
        			getpid(),WIFEXITED (wait_status),WEXITSTATUS (wait_status));
        if (errno != 0) {
            perror ("wait");
            syslog(LOG_USER, "Assignment 3-12, do_exec:     parent, pid = %d, errno = %d, returning false",getpid(),errno);
            return false; }
        else if (pid == -1) {
            perror ("wait");
            syslog(LOG_USER, "Assignment 3-13, do_exec:     parent, wait error, pid = %d, returning false",getpid());
            return false; }
        else if (WIFSIGNALED (wait_status)) {
            syslog(LOG_USER, "Assignment 3-16, do_exec:     parent, pid = %d, Killed by signal = %d%s",getpid(),WTERMSIG (wait_status), 
                        	WCOREDUMP (wait_status) ? " (dumped core)" : "");
            return false; }
        else if (WIFSTOPPED (wait_status)) {
            syslog(LOG_USER, "Assignment 3-17, do_exec:     parent, pid = %d, Stopped by signal = %d",getpid(),
            WSTOPSIG (wait_status));
            return false; }
        else if (WIFCONTINUED (wait_status)) {
            syslog(LOG_USER, "Assignment 3-18, do_exec:     parent, pid = %d, Continued.",getpid());
            return false; }
        else if (WIFEXITED (wait_status)) 
        {
            syslog(LOG_USER, "Assignment 3-21, do_exec:     parent, pid = %d, Normal termination with exit status = %d",getpid(),
                         WEXITSTATUS (wait_status));
            if (WEXITSTATUS (wait_status) == 0) {
                syslog(LOG_USER, "Assignment 3-22, do_exec:     parent, pid = %d, returning true",getpid());
                return true;   }
                //return false;   }
            else {
                syslog(LOG_USER, "Assignment 3-23, do_exec:     parent, pid = %d, returning false",getpid());
                return false;  }
        }
      }  
    return false;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    int status; 
    int fd = -1; 
    struct stat stats; 
    syslog(LOG_USER, "Assignment 3-24,   do_exec_redirect: start of function");
    
    int result = stat(command[0], &stats); 
    if (result != 0 && errno == ENOENT) { 
        syslog(LOG_USER, "Assignment 3-25, do_exec_redirect: redirect path incorrect, %s",command[0]);
        va_end(args); 
        return false; } 
    if (count < 1) {
    syslog(LOG_ERR, "Assignment3-26, do_exec_redirect: not enough arguments"); 
        va_end(args); 
        return false; } 
        
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    va_end(args);
    
    fd = open(outputfile, O_WRONLY|O_CREAT, 0644); 
    
    if (fd < 0) { 
        return false;
        abort(); } 
    syslog(LOG_USER, "Assignment 3-28,   do_exec_redirect: open %s",outputfile);

    int kidpid;
    //int fd = open("redirected.txt", O_WRONLY|O_TRUNC|O_CREAT, 0644);
    // Source - https://stackoverflow.com/a/13784315
    // Posted by tmyklebu, modified by community. See post 'Timeline' for change history
    // Retrieved 2026-06-01, License - CC BY-SA 3.0
    
    switch (kidpid = fork()) {
      case -1: 
        perror("fork"); 
        syslog(LOG_USER, "Assignment 3-30,  do_exec_redirect: case = -1, fork error, pid = %d",getpid());
        exit(EXIT_FAILURE);
      case 0:
        //child process
        if (dup2(fd, 1) < 0) { 
            perror("dup2"); 
            syslog(LOG_USER, "Assignment 3-29,  do_exec_redirect: dup2 error"); 
            exit(EXIT_FAILURE);
            abort(); }
        close(fd);
        
        syslog(LOG_USER, "Assignment 3-31,   do_exec_redirect: parent, pid = %d, path = %s",getpid(),command[0]);
        for(i=0; i<count+1; i++) {  //put command in syslog
          syslog(LOG_USER, "Assignment 3-32,   do_exec_redirect: child, pid = %d, command[%d] = %s",getpid(),i,command[i]);  }    
        if (execv(command[0], command) == -1)   {
            syslog(LOG_USER, "Assignment 3-33, do_exec_redirect: child, pid = %d, should not be here",getpid());  
            exit(EXIT_FAILURE);  } //exec failed
        syslog(LOG_USER, "Assignment 3-35,  do_exec_redirect: execv error, pid = %d",getpid());
        //perror("execv"); 
        exit(EXIT_FAILURE);
      default:
        close(fd);
        /* do whatever the parent wants to do. */
        if (waitpid (-1, &status, 0) == -1) {  
           syslog(LOG_USER, "Assignment 3-36,  do_exec_redirect: parent, waitpid error, pid = %d",getpid());
           return false; }  
        else if (WIFSIGNALED (status)) {
            syslog(LOG_USER, "Assignment 3-38, do_exec_redirect: parent, pid = %d, Killed by signal = %d%s",getpid(),WTERMSIG (status), 
                        WCOREDUMP (status) ? " (dumped core)" : "");
            return false; }
        else if (WIFSTOPPED (status)) {
            syslog(LOG_USER, "Assignment 3-39, do_exec_redirect: parent, pid = %d, Stopped by signal = %d",getpid(),
            WSTOPSIG (status));
            return false; }
        else if (WIFCONTINUED (status)) {
            syslog(LOG_USER, "Assignment 3-40, do_exec_redirect: parent, pid = %d, Continued.",getpid());
            return false; }
        else if (WIFEXITED (status)) 
        {
            syslog(LOG_USER, "Assignment 3-45,   do_exec_redirect: parent, pid = %d, Normal termination with exit status = %d",getpid(),
                         WEXITSTATUS (status));
            if (WEXITSTATUS (status) == 0) {
                syslog(LOG_USER, "Assignment 3-46,   do_exec_redirect: parent, pid = %d, Returning true, count = %d",getpid(),count);
                return true;   }
            else {
                syslog(LOG_USER, "Assignment 3-47, do_exec_redirect: parent, pid = %d, Returning false)",getpid());
                return false; }
        }
    return false;
    }
}

