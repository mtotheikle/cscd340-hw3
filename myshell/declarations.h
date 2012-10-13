


#ifndef tester2_linkedList2_h
#define tester2_linkedList2_h

struct Job {
	char *command;
    
    int isPiped;
    int wasPiped;
    
    int alias;
    
    int redirectOut;
    int redirectOutOverwrite;
    
    int redirectIn;
    int redirectInOverwrite;
    
	struct Job *next;
    struct Job *prev;
};

typedef struct Job Job;

Job *head;
int size;

#endif