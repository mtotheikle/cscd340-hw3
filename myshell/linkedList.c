//
//  linkedList.c
//  Michael Williams
//  HW 4
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkedList.h"

int size = 0;

Node * createNode(char *aliasName, char *aliasCommand)
{
    Node * node = (Node *) malloc(sizeof(Node));
    node->next = NULL;
    
    node->aliasName = calloc(strlen(aliasName), sizeof(char *));
    strcpy(node->aliasName, aliasName);
    
    if (aliasCommand != NULL) {
        node->aliasCommand = calloc(strlen(aliasCommand), sizeof(char *));
        strcpy(node->aliasCommand, aliasCommand);    
    }
    
    return node;    
}

void freeNode(Node *node)
{
    free(node->aliasName);
    free(node->aliasCommand);
    free(node);
    node = NULL;
}

void addOrdered(Node * node)
{
    if (findAliasNode(node->aliasName)) {
        deleteAlias(node->aliasName);
    }
    
    size++;
    
    if (head == NULL) {
        head = node;
    } else {
        
        if (strcmp(head->aliasName, node->aliasName) > 0) {
            node->next = head;
            head = node;
            
            return;
        }
        
        Node *cur = head;
        Node *prev;
        while (strcmp(cur->aliasName, node->aliasName) < 0) {
            prev = cur;
            
            if (cur->next == NULL) {
                cur->next = node;
                
                return;
            }
            
            cur = cur->next;
        }
        
        node->next = cur;
        prev->next = node;
    }
}

void deleteNodeAt(int i)
{
    if (i == 0) {
        // Removing head
        Node *tmp = head;
        head = head->next;
        
        tmp->next = NULL;
        freeNode(tmp);
    } else {
        int j = 0;
        Node *node = head;
        Node *prev = NULL;
        while (j < i) {
            prev = node;
            node = node->next;
            j++;
        }
                
        prev->next = node->next;
        node->next = NULL;
        freeNode(node);
    }
    
    size--;
}

void printNodeValues(Node *n)
{
    printf("alias %s='%s'\n", n->aliasName, n->aliasCommand);
}

void printAliases()
{
    if (size == 0) {
        return;
    }
    
    Node *n = head;
    while (n != NULL) {
        printNodeValues(n);
        n = n->next;
    }    
}

void printNodeAt(int nodeNumber)
{
    if (head == NULL) {
        return;
    }
    
    int i;
    Node * n = head;
    for (i = 0; i < nodeNumber; i++) {
        n = n->next;
    }
    
    printNodeValues(n);
}

int findAliasIndex(char * aliasName)
{
    Node *tmp = head;
    int i = 0;
    while (tmp != NULL) {
        if (strcmp(tmp->aliasName, aliasName) == 0) {
            return i;
        }
        
        i++;
        tmp = tmp->next;
    }
    
    return -1;
}

Node * findAliasNode(char *aliasName)
{
    Node *tmp = head;
    int i = 0;
    while (tmp != NULL) {
        if (strcmp(tmp->aliasName, aliasName) == 0) {
            return tmp;
        }
        
        i++;
        tmp = tmp->next;
    }
    
    return NULL;
}

void deleteAlias(char *aliasName)
{    
    if (size == 0) {
        return;
    }
    
    int index = findAliasIndex(aliasName);
    
    if (index != -1) {
        deleteNodeAt(index);
    }
}

void clearAliases()
{
    Node * tmp = head;
	while (tmp != NULL) {
        tmp = tmp->next;
        deleteNodeAt(0);
    }
    
    head = NULL;
}