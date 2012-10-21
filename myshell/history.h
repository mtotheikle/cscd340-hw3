//
//  history.h
//  myshell
//
//  Created by Michael Williams on 10/15/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef myshell_history_h
#define myshell_history_h

typedef struct HistoryNode {
	Job *job;
	struct HistoryNode *next;
} HistoryNode;

HistoryNode *historyHead;
int historySize;

void printHistory();

void addHistory(Job *job);

void readHistory();

void cleanHistory();

HistoryNode *getHistoryCommand(int index);

#endif
