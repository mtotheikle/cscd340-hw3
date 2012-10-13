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
#include "linkedList.h"
#define MAX 1000

// tests:
// ls -la;
// ls ; 
// ls -la; which test
// ls -la; which test;
// ls -la; which

Node *head = NULL;

// Notes bash syntax for redirecting file descriptors to another does not work, for example: 2>&1
// Notes: Alias with pipe does not work
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
    char *command = strdup(s);
	char * token = strtok(command, " ");
    i = 0;
    while (token != NULL) {
        (*argv)[i++] = strdup(token);
        token = strtok(NULL, " ");
    }
    
    // Ensure the last argument has a null
    (*argv)[i] = NULL;
    
    return count;
    
}// end makeArgs

void makeAlias(char *command)
{
    int i = 5;
    char *aliasName = NULL;
    char *aliasCommand = NULL;
    
    int aliasNameBegin = 0, aliasNameEnd = 0;
    int commandBegin = 0, commandEnd = 0;
    char commandStartChar;
    
    while (i < strlen(command)) {
        char c = command[i];
        
        if (c == ' ') {
            // Keep going until we find a character
        } else if (c == '=') {
            // Next character should be '"'
            aliasNameEnd = i - 1;
            
        } else if (c == '"' || c == '\'') {
            if (commandBegin == 0) {
                commandBegin = i+1;
                commandStartChar = c;
            } else if (commandStartChar == c) {
                commandEnd = i - 1;
            }
        } else {
            if (aliasNameBegin == 0) {
                // This is a character
                aliasNameBegin = i;
            }
        }
        i++;
    }
    
    if (!aliasNameEnd) {
        aliasNameEnd = i; // They just provided and alias name
    }
    i = 0;
        
    aliasName = calloc((aliasNameEnd - aliasNameBegin) + 2, sizeof(char *));
    strncpy(aliasName, command + aliasNameBegin, (aliasNameEnd - aliasNameBegin) + 1);
    
    if (commandBegin == 0) {
        aliasCommand = NULL;
    } else {
        aliasCommand = calloc((commandEnd - commandBegin) + 2, sizeof(char *));
        strncpy(aliasCommand, command + commandBegin, (commandEnd - commandBegin) + 1);
    }
    
    if (aliasCommand) { // Saving alias
        Node *alias = createNode(aliasName, aliasCommand);
        addOrdered(alias);
    } else { // Describe alias
        
    }
}

void handleAlias(char *command)
{
    if (strlen(command) == 5) {
        // No arguements, display aliases
        printAliases();
    } else {
        makeAlias(command);
    }
}

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
    
    // If we have alias then we need to use that instead
    Node *alias1 = findAliasNode(args1[0]);
    if (alias1 != NULL) {
        clean(argsc1, args1);
        argsc1 = makeargs(alias1->aliasCommand, &args1);
    }
    
    Node *alias2 = findAliasNode(args2[0]);
    if (alias2 != NULL) {
        clean(argsc2, args2);
        argsc2 = makeargs(alias2->aliasCommand, &args2);
    }
    
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
    
    if (argsc > 0)
    {
        if (strcmp(args[0], "alias") == 0) {
            handleAlias(command);
        } else {
            
            // If we have alias then we need to use that instead
            Node *alias = findAliasNode(args[0]);
            if (alias != NULL) {
                clean(argsc, args);
                argsc = makeargs(alias->aliasCommand, &args);
            }
            
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
    }
}

int runJobs(Job * job)
{
    while (job != NULL) {
        
        if (goAgain(job->command) == 0) {
            return 0;
        }
        
        if (job->isPiped && job->next) {
            
            if (strcmp(job->next->command, "") == 0) {
                printf("invalid syntax");
                
                return 1;
            }
            
            forkMe(&job->command, &job->next->command);
            job = job->next; // Advance to next job as we have already ran the next one
        } else {
            runCommand(job->command);
        }
        
        job = job->next;
    }
    
    return 1;
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
    int j;
    char buf[MAX];
    char c = getchar();
    Job *job = NULL;

    int inQuote = 0;
    char quoteChar;
    
    while (1) {
        switch (c)
        {
            case ';':
                
                if (inQuote) {
                    buf[i] = c; // Add character to input
                    i++; // Increment pointer
                    c = getchar();
                    
                    continue;
                }
                
                buf[i] = '\0';
                if (job == NULL) {
                    job = createJob(buf);
                } else {
                    job->next = createJob(buf);
                    job->next->prev = job;
                    job = job->next;
                }
                
                for (j = i; j >= 0; j--) {
                    buf[j] = '\0';
                }                
                i = 0;
            break;
                
            case '>':
                
                if (inQuote) {
                    buf[i] = c; // Add character to input
                    i++; // Increment pointer
                    c = getchar();
                    
                    continue;
                }
                
                if (job == NULL) {
                    buf[i] = '\0';
                    job = createJob(buf);
                } else {
                    job->next = createJob(buf);
                    job->next->prev = job;
                    job = job->next;
                }
                
                // This is just redirection and could build
                if (job->redirectOut == 1) {
                    job->redirectOut = 0;
                    job->redirectOutOverwrite = 1;
                }
                
                break;
            case '<':
                
                if (inQuote) {
                    buf[i] = c; // Add character to input
                    i++; // Increment pointer
                    c = getchar();
                    
                    continue;
                }
                
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
                
            case '|': // Our favorie, piping!
                
                if (inQuote) {
                    buf[i] = c; // Add character to input
                    i++; // Increment pointer
                    c = getchar();
                    
                    continue;
                }
                
                if (job == NULL) {
                    buf[i] = '\0';
                    job = createJob(buf);
                } else {
                    job->next = createJob(buf);
                    job->next->prev = job;
                    job = job->next;
                }
                
                job->isPiped = 1;
                
                if (job->prev != NULL) {
                    job->prev->wasPiped = 1;
                }                
                
                buf[i] = '\0';
                for (j = i; j > 0; j--) {
                    buf[j] = '\0';
                }                
                i = 0;
            break;
                
            case '\n':
                if (buf[0] != '\0') { // Ensure we don't have an empty buffer
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
                }
                
                for (j = i; j >= 0; j--) {
                    buf[j] = '\0';
                }                
                i = 0;
                
                return job;
            break;
                
            case ' ':
                if (i != 0) {
                    buf[i] = c; // Add character to input
                    i++; // Increment pointer
                }
            break;
                
            case '"':
                if (inQuote && quoteChar == c) {
                    inQuote = 0;
                } else {
                    inQuote = 1;
                }
                
                quoteChar = c;
                buf[i] = c; // Add character to input
                i++; // Increment pointer
            break;
                
            case '\'':
                if (inQuote && quoteChar == c) {
                    inQuote = 0;
                } else {
                    inQuote = 1;
                }
                
                quoteChar = c;
                buf[i] = c; // Add character to input
                i++; // Increment pointer                
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
    int run = 1;
    while (run == 1) {        
        job = getJobs();
        
        run = runJobs(job);
    }
    
    clearAliases();
    
    return 0;
}

