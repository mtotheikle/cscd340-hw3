//
//  main.c
//  Created by Michael Williams on 10/12/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <unistd.h>
#include "declarations.h"
#define MAX 1000

// tests:
// ls -la; which test
// ls -la; which test;
// ls -la; which

void strip(char * s)
{
	int len = strlen(s);
    
	if(s[len - 2] == '\r')
		s[len - 2] = '\0';
    
	else if(s[len - 1] == '\n')
		s[len - 1] = '\0';
}// end strip

int makeargs(char *s, char *** argv)
{
    // http://www.cplusplus.com/reference/clibrary/cstring/strtok/
    int length = (int) strlen(s);
    if (length < 1) {
        return -1; // no args
    }
    
    int i;
    int count = 1; // count starts at 1 as we assume there is always one arguemnt
    for (i = 0; i < length; i++) {
        if (s[i] == ' ') {
            count++;
        }
    }
    
    *argv = calloc(count + 1, sizeof(char*));
	char * token = strtok(s, " ");
    i = 0;
    while (token != NULL) {
        (*argv)[i++] = strdup(token);
        token = strtok(NULL, " ");
    }
    
    // Ensure the last argument has a null
    (*argv)[i] = NULL;
    
    return count;
    
}// end makeArgs

void clean(int argc, char **argv)
{
    int i;
    for (i = 0; i < argc; i++) {
        free(argv[i]);
        argv[i] = NULL;
    }
    
    free(argv);
}

void printargs(int argc, char **argv)
{
	int x;
	for(x = 0; x < argc; x++)
		printf("%s\n", argv[x]);
}

void forkMe(char **left, char **right)
{
    char **args1 = NULL;
    int argsc1 = makeargs(*left, &args1);
    
    char **args2 = NULL;
    int argsc2 = makeargs(*right, &args2);
    
    if (fork() != 0)
    {
        int status;
        waitpid(-1, &status, 0);
        
        clean(argsc1, args1);
        clean(argsc2, args2);
    }
    else {
        
        int fd[2];
        // If it does not work a -1 will be returned
        pipe(fd);
        
        pid_t pid = fork();
        if (pid == -1) 
            return;
        
        if (pid != 0)  
        {  
            close(fd[0]); //close read from pipe, in parent
            dup2(fd[1], STDOUT_FILENO); // Replace stdout with the write end of the pipe
            close(fd[1]); // Don't need another copy of the pipe write end hanging about
            execvp(args1[0], args1);
            
            // Somethign failed, bail
            exit(0);
        }
        else
        {
            close(fd[1]); //close write to pipe, in child
            dup2(fd[0], STDIN_FILENO); // Replace stdin with the read end of the pipe
            close(fd[0]); // Don't need another copy of the pipe read end hanging about
            execvp(args2[0], args2);
            
            // Something failed, bail
            exit(0);
        }
    }      
}

int goAgain(char * s)
{
    if (strcmp(s, "exit") == 0)
        return 0;
    
    return 1;
    
}// end goAgain

void runCommand(char * command)
{
    char **args = NULL;
    int argsc = makeargs(command, &args);
    
    if (fork() != 0)
    {
        int status;
        waitpid(-1, &status, 0);
        
        clean(argsc, args);
    }
    else {
        execvp(args[0], args);
        
        printf("Invalid command %s\n", command);
        
        // Something failed, bail
        exit(0);
    }
}

void runJobs(Job * job)
{
    while (job != NULL) {
        runCommand(job->command);
        
        job = job->next;
    }
}
    
Job * createJob(char *command)
{
    Job *job = (Job *) malloc(sizeof(Job));
    job->isPiped = 0;
    job->wasPiped = 0;
    job->redirectOut = 0;
    job->redirectOutOverwrite = 0;
    job->alias = 0;
    
    job->prev = NULL;
    job->next = NULL;
    
    job->command = malloc(sizeof(char *) * strlen(command));
    strcpy(job->command, command);
        
    return job;
}

Job * getJobs()
{    
    printf("?:");
    
    int i = 0;
    char buf[MAX];
    char c = getchar();
    Job *job = NULL;

    while (1 == 1) {
        switch (c)
        {
            case ';':
                
                buf[i] = '\0';
                if (job == NULL) {
                    job = createJob(buf);
                } else {
                    job->next = createJob(buf);
                    job = job->next;
                }
                
                buf[i] = '\0';
                int j;
                for (j = i; j > 0; j--) {
                    buf[j] = '\0';
                }                
                i = 0;
            break;
                
            case '>':
                if (job == NULL) {
                    buf[i] = '\0';
                    job = createJob(buf);
                }
                
                // This is just redirection and could build
                if (job->redirectOut == 1) {
                    job->redirectOut = 0;
                    job->redirectOutOverwrite = 1;
                }
                
                break;
            case '<':
                
                if (job == NULL) {
                    buf[i] = '\0';
                    job = createJob(buf);
                }
                
                // Again redirection and can build
                if (job->redirectIn == 1) {
                    job->redirectIn = 0;
                    job->redirectInOverwrite = 1;
                }
                
                break;
            
            case '=':
                // Alias command
                
                break;
                
            case '|': // Our favorie, piping!
                
                if (job == NULL) {
                    buf[i] = '\0';
                    job = createJob(buf);
                } else {
                    job->next = createJob(buf);
                    job = job->next;
                }
                
                job->isPiped = 1;
                
                if (job->prev != NULL) {
                    job->prev->wasPiped = 1;
                }
            break;
                
            case '\n':
                if (job == NULL) {
                    buf[i] = '\0';
                    job = createJob(buf);
                } else {
                    job->next = createJob(buf);
                    job->next->prev = job;
                    job = job->next;
                }
                
                buf[i] = '\0'; // Add null terminator to buf        
                
                // Get the job pointer to the very first job
                while (job->prev != NULL) {
                    job = job->prev;
                }
                
                return job;
            break;
                
            default:
                buf[i] = c; // Add character to input
                i++; // Increment pointer
            break;
        }
        
        c = getchar();
    }
}

int main(int argc, const char * argv[])
{
    system("clear");
    
    Job * job = NULL;
    job = getJobs();
    
    while (1 == 1) {
        runJobs(job);
        
        job = getJobs();
    }
    
    
    return 0;
}

