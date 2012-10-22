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
    if (job->command == NULL) {
        return;
    }
    
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
    return;
    
    // @todo Free job
    //freeJob(node->job);
    free(node);
    node = NULL;
}

void deleteHistoryNodeAt(int i)
{
    if (i == 0) {
        // Removing head
        HistoryNode *tmp = historyHead;
        historyHead = historyHead->next;
        
        tmp->next = NULL;
        freeHistoryNode(tmp);
    } else {
        int j = 0;
        HistoryNode *node = historyHead;
        HistoryNode *prev = NULL;
        while (j < i) {
            prev = node;
            node = node->next;
            j++;
        }
        
        prev->next = node->next;
        node->next = NULL;
        freeHistoryNode(node);
    }
    
    historySize--;
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
    
    int i = 1;
    int maxFileLines = atoi(getenv("HISTFILESIZE"));
    int numToSkip = 0;
    if (maxFileLines < historySize) {
        // We need to restrict what we write to the file
        numToSkip = historySize - maxFileLines;
    }
    
    while (cur != NULL) {
        tmp = cur;
        cur = cur->next;
        
        if (i <= numToSkip) {
            i++;
        } else {
            printJob(tmp->job, fd);
        }
                
        deleteHistoryNodeAt(0);
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