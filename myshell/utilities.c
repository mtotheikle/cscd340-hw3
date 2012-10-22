//
//  utilities.c
//  myshell
//
//  Created by Michael Williams on 10/15/12.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "declarations.h"
#include "linkedList.h"

void strip(char * s)
{
	int len = strlen(s);
    
	if(s[len - 2] == '\r')
		s[len - 2] = '\0';
    
	else if(s[len - 1] == '\n')
		s[len - 1] = '\0';
}// end strip

void clean(int argc, char **argv)
{
    int i;
    for (i = 0; i < argc; i++) {
        free(argv[i]);
        argv[i] = NULL;
    }
    
    free(argv);
    argv = NULL;
}

int makeargs(char *s, char *** argv)
{
    // Reference http://www.cplusplus.com/reference/clibrary/cstring/strtok/
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
    
    *argv = malloc((count + 1) * sizeof(char*));
    char *command = strdup(s);
	char * token = strtok(command, " ");
    
    i = 0;
    while (token != NULL) {
        (*argv)[i++] = strdup(token);
        token = strtok(NULL, " ");
    }
    
	free(command);    
    
    // Ensure the last argument has a null
    (*argv)[i] = NULL;
    
    return count;
    
}// end makeArgs

void freeJob(Job *job)
{
    if (job->command != NULL) {
        free(job->command);
        job->command = NULL;
    }
    
    if (job->inFile != NULL) {
        free(job->inFile);
        job->inFile = NULL;    
    }
    
    if (job->outFile != NULL) {
        free(job->outFile);
        job->outFile = NULL;    
    }
    
    free(job);
    job = NULL;
}

void cleanJobs(Job * job)
{
    Job * tmp;
    while (job != NULL) {
        tmp = job->next;
        freeJob(job);
        job = tmp;
    }
}

void clearBuffer(char *buf)
{
    if (buf == NULL) {
        return;
    }
    
    int i;
    int len = strlen(buf);
    for (i = 0; i < len; i++) {
        buf[i] = '\0';
    }
}