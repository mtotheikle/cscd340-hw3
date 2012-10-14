// Michael Williams 
// Homework 3


#ifndef tester2_linkedList_h
#define tester2_linkedList_h

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

void printAliases();

void clearAliases();

#endif
