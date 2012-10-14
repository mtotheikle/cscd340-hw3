


#ifndef tester2_linkedList2_h
#define tester2_linkedList2_h

typedef struct Job {
	char *command;
    char *inoutFile;
    
    int isPiped;
    int wasPiped;
    
    int alias;
    
    int redirectOut;
    int redirectOutAppend;
    int redirectIn;
    
	struct Job *next;
    struct Job *prev;
} Job;

#endif