//
//  linkedList.h
//  Michael Williams
//  HW 3
//


#ifndef linkedList_h
#define linkedList_h

#include <stdio.h>

typedef struct Node {
	char *aliasName;
	char *aliasCommand;
	struct Node *next;
} Node;

Node *head;
int size;

Node * createNode(char *aliasName, char *aliasCommand);

Node * findAliasNode(char *aliasName);

void addOrdered(Node * node);

void deleteAlias(char *aliasName);

int findAliasIndex(char * aliasName);

void printNodeAt(int nodeNumber);

void printAliases();

void clearAliases();

#endif
