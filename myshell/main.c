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

int goAgain(char * s)
{
    if (strcmp(s, "exit") == 0)
        return 0;
    
    return 1;
    
}// end goAgain

int runJobs(Job * job)
{
    int numcmds = 0;
    Job *tmp = job;
    while (job != NULL) {
        numcmds++;
        job = job->next;
    }
    job = tmp;
    
//    int newstdin = dup(STDIN_FILENO);
    
    int reading[numcmds];
    int writing[numcmds];
    int argsc;
    char **args;
    
    int p;
    for(p=0; p < numcmds; p++)
    {
        reading[p] = -1;
        writing[p] = -1;
    }
    
    int j;
    for(j=0; j < numcmds-1; j++)
    {
        int fileds[2];
        pipe(fileds);
        reading[j+1] = fileds[0]; // Reading end of pipe
        writing[j] = fileds[1]; // Writing end of pipe
    }
    
    int i = 0;
    while (job != NULL) {
    
        // Parse command
        argsc = makeargs(job->command, &args);
        
        // See if we can find an alias matching the command
        Node *alias1 = findAliasNode(args[0]);
        if (alias1 != NULL) {
            clean(argsc, args);
            argsc = makeargs(alias1->aliasCommand, &args);
        }
        
        pid_t childpid;
        int status;
        childpid=fork();
        
        if (childpid >= 0) 
        {
            if (childpid == 0) 
            {
                // This is the child process
                // TESTING
//                dprintf(newstdin, "Command '%s'\n", job->command);
//                dprintf(newstdin, "writing[%d] = %d\n", i, writing[i]);
//                dprintf(newstdin, "reading[%d] = %d\n", i, reading[i]);
                
                if (writing[i] != -1)
                {
                    close(STDOUT_FILENO);
                    dup2(writing[i],STDOUT_FILENO);
                }
                
                if (reading[i] != -1)
                {
                    close(0);
                    dup2(reading[i],0);
                }
                
                if (execvp(args[0], args) == -1) 
                {
                    perror("problem with command");
                    exit(0);
                }
            }
            else 
            {
                // parent
                wait(&status);
                close(writing[i]);
                
                if(i > 0) 
                {
                    close(reading[i]);
                }
            }
        }
        else 
        {
            perror("fork");
        }
        
        i++;
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
                        buf[i] = '\0'; // Add null terminator to buf 
                        job->next = createJob(buf);
                        job->next->prev = job;
                        job = job->next;
                    }
                    
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

