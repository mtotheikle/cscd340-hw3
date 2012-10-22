//
//  history.c
//  myshell
//
//  Created by Michael Williams on 10/15/12.
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
#include "utilities.h"

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
    cleanJobs(node->job);
    node->next = NULL;
    node->job = NULL;
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
    if (historySize == 0) {
        return;
    }
    
    FILE * fd = fopen(HISTFILE, "wb");
    
    int maxFileLines = 100; // Default to 100
    if (getenv("HISTFILESIZE")) {
        maxFileLines = atoi(getenv("HISTFILESIZE"));
    }
    
    int numToSkip = 0;
    if (maxFileLines < historySize) {
        // We need to restrict what we write to the file
        numToSkip = historySize - maxFileLines;
    }
    
    int i = 1;
    HistoryNode *toDelete = NULL;
    HistoryNode *cur = historyHead;    
    while (cur != NULL) {
        toDelete = cur;
        cur = cur->next; // advance to next node
        if (i > numToSkip) {
            printJob(toDelete->job, fd); // print job to file
        }
        i++;
                
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