//
//  history.c
//  myshell
//
//  Created by Michael Williams on 10/15/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include "stdlib.h"
#include "declarations.h"
#include "history.h"

HistoryNode * createHistoryNode(Job *job)
{
    HistoryNode * node = (HistoryNode *) malloc(sizeof(HistoryNode));
    node->next = NULL;
    node->job = job;

    return node;    
}

void addHistory(Job *job)
{
    historySize++;
    
    HistoryNode *node = createHistoryNode(job);
    
    if (historyHead == NULL) {
        historyHead = node;
    } else {
        HistoryNode *tmp = historyHead;
        
        while (tmp->next != NULL) {
            tmp = tmp->next;
        }
        
        tmp->next = node;
    }
}

void freeHistoryNode(HistoryNode *node)
{
    // @todo Free job
    //freeJob(node->job);
    free(node);
    node = NULL;
}

void printJob(Job * job, FILE * fd)
{
    Job *tmp = job;
    while (tmp != NULL) {
        
        if (job->redirectIn) {
            fprintf(fd, "%s < ", job->inFile); 
        }
        
        fprintf(fd, "%s", job->command);
        
        if (job->isPiped) {
            fprintf(fd, " | ");
        }
        
        if (job->redirectOut) {
            fprintf(fd, " > %s", job->outFile);
        }
        
        if (job->redirectOutAppend) {
            fprintf(fd, " >> %s", job->outFile);
        }
        
        fprintf(fd, "\n");
        
        tmp = job->next;
    }
}

void cleanHistory()
{
    HistoryNode *tmp;
    HistoryNode *cur = historyHead;
    FILE * fd = fopen(HISTFILE, "wb");
    while (cur != NULL) {
        printJob(cur->job, fd);
        tmp = cur;
        cur = cur->next;
        freeHistoryNode(tmp);
    }
    
    fclose(fd);
}

void printHistory()
{
    HistoryNode *tmp = historyHead;
    int i = 1;
    while (tmp != NULL) {
        
        printf("\t %d ", i++);
        printJob(tmp->job, stdout);
        
        tmp = tmp->next;
    }   
}

HistoryNode *getHistoryCommand(int index)
{
    if (index > historySize || index < 0) {
        return NULL;
    }
    
    int i;
    HistoryNode *tmp = historyHead;
    for (i = 0; i < index; i++) {
        tmp = tmp->next;
    }
    
    return tmp;
}