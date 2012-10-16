//
//  declarations.h
//  Michael Williams
//  HW 3
//

// These are declarations used in the main program, not the linked list piece
#ifndef declarations_h
#define declarations_h

#define MAX 80
#define DEBUG 0

typedef struct Job {
	char *command;
    char *inFile;
    char *outFile;
    
    int isPiped;
    int alias;
    
    int redirectOut;
    int redirectOutAppend;
    int redirectIn;
    
	struct Job *next;
    struct Job *prev;
} Job;

#endif