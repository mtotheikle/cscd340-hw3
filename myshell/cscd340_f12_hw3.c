//
//  main.c
//  Michael Williams
//  HW 3
//

// 
// Note: Bash syntax for redirecting file descriptors to another does not work, for example: 2>&1
// Note: Alias with pipes, redirection or multiple commands does not work. Aliases are only supporting
//       simple 1 command aliases
// Note: makeargs was not specified to handle quotes, therefore running a command such as grep "test cool" 
//       will not pass arguements correctly and therefore unexpected behavior with grep will happen
// Note: Git was used to track this code so others may have found it but it was not made available publicly until
//       10/14/12 - https://github.com/mtotheikle/cscd340-hw3
// Note: There is little input validation so inputing something like alias la='ls -a 
//       will cause it to break as getchar() is called and will not know there is no more input

// OLD PATH: 
// /Users/Mike/.rvm/gems/ruby-1.9.3-p0/bin:/Users/Mike/.rvm/gems/ruby-1.9.3-p0@global/bin:/Users/Mike/.rvm/rubies/ruby-1.9.3-p0/bin:/Users/Mike/.rvm/bin:/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/X11/bin:/opt/local/bin:/usr/local/git/bin

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include "declarations.h"
#include "linkedList.h"
#include "utilities.h"
#include "history.h"

#include <signal.h>

// tests:
// ls -la;
// ls ; 
// ls -la; which test
// ls -la; which test;
// ls -la; which
// grep main < main.c > realout.txt

// ls -la >> test.txt; cat test.txt | less

Node *head = NULL;
HistoryNode *historyHead = NULL;

int runJobs(Job * job, int addToHistory);

void makeAlias(char *command)
{
    int i = 5;
    char *aliasName = NULL;
    char *aliasCommand = NULL;
    
    int aliasNameBegin = 0, aliasNameEnd = 0;
    int commandBegin = 0, commandEnd = 0;
    char commandStartChar;
    
    while (i < strlen(command)) {
        char c = command[i];
        
        if (c == ' ') {
            // Keep going until we find a character
        } else if (c == '=') {
            // Next character should be '"'
            aliasNameEnd = i - 1;
            
        } else if (c == '"' || c == '\'') {
            if (commandBegin == 0) {
                commandBegin = i+1;
                commandStartChar = c;
            } else if (commandStartChar == c) {
                commandEnd = i - 1;
            }
        } else {
            if (aliasNameBegin == 0) {
                // This is a character
                aliasNameBegin = i;
            }
        }
        i++;
    }
    
    if (!aliasNameEnd) {
        aliasNameEnd = i; // They just provided and alias name
    }
        
    aliasName = calloc((aliasNameEnd - aliasNameBegin) + 2, sizeof(char *));
    strncpy(aliasName, command + aliasNameBegin, (aliasNameEnd - aliasNameBegin) + 1);
    
    if (commandBegin == 0) {
        aliasCommand = NULL;
    } else {
        aliasCommand = calloc((commandEnd - commandBegin) + 2, sizeof(char *));
        strncpy(aliasCommand, command + commandBegin, (commandEnd - commandBegin) + 1);
    }
    
    if (aliasCommand) { // Saving alias
        Node *alias = createNode(aliasName, aliasCommand);
        addOrdered(alias);
    } else { // Describe alias
        int index = findAliasIndex(aliasName);
        if (index != -1) {
            printNodeAt(index);
        }
    }
    
    free(aliasName);
    if (aliasCommand != NULL) {
        free(aliasCommand);
    }
}

void handleAlias(char *command)
{
    if (strlen(command) == 5) {
        // No arguements, display aliases
        printAliases();
    } else {
        makeAlias(command);
    }
}

void printargs(int argc, char **argv)
{
	int x;
	for(x = 0; x < argc; x++)
		printf("%s\n", argv[x]);
}

int checkBuiltInCommands(Job *job, char **args, int argsc, int addToHistory)
{
    if (strcmp(args[0], "alias") == 0) {
        if (addToHistory) {
            addHistory(job);
        }
        
        handleAlias(job->command);
        
        job = job->next;
        
        return 1;
    } else if (strcmp(args[0], "unalias") == 0) {
        if (addToHistory) {
            addHistory(job);
        }
        
        deleteAlias(args[1]);
        
        job = job->next;
        
        return 1;
    } else if (strcmp(args[0], "history") == 0) {
        if (addToHistory) {
            addHistory(job);
        }
        
        printHistory();
        
        return 1;
    } else if (strcmp(args[0], "cd") == 0) {
        
        if (addToHistory) {
            addHistory(job);
        }
        
        // @todo Support ~
        int result = chdir(args[1]);
        
        if (result == -1) {
            printf("cd: %s: No such file or directory", args[1]);
        }
        
        return 1;
    } else if (args[0][0] == '!') {
        
        strsep(&args[0], "!"); // Once to remove the "!"
        int num = atoi(strsep(&args[0], "!")); // Anothoer time to get the number
        HistoryNode *his = getHistoryCommand(num - 1);
        if (his != NULL) {
            runJobs(his->job, 0);
        }
        
        return 1;
        
    } else if (strcmp(args[0], "!!") == 0) {
        
        HistoryNode *his = getHistoryCommand(historySize - 1);
        if (his != NULL) {
            runJobs(his->job, 0);
        }
        
        return 1;
    }
    
    return -1;
}

// Some code was used and modifed from http://stackoverflow.com/questions/1694706/problem-with-piping-commands-in-c
int runJobs(Job * job, int addToHistory)
{
    int numcmds = 0;
    Job *tmp = NULL;
    
    tmp = job;
    while (tmp != NULL) {
        numcmds++;
        tmp = tmp->next;
    }
    
#if DEBUG
    // Keep copy 
    int newstdin = dup(STDIN_FILENO);
#endif
    
    int reading[numcmds];
    int writing[numcmds];
    
    int argsc;
    char **args;
    
    int p;
    for (p=0; p < numcmds; p++)
    {
        reading[p] = -1;
        writing[p] = -1;
    }
    
    // Now we need to figure out how we need to pipe the output of the command
    tmp = job; // Assign to tmp so we can loop multiple times
    int fileds[2];
    int j;
    for (j = 0; j < numcmds; j++)
    {
        if (tmp->isPiped && j < numcmds - 1) {
            pipe(fileds);
            reading[j+1] = fileds[0]; // Reading end of pipe
            writing[j] = fileds[1]; // Writing end of pipe
        }
        
        if (tmp->redirectOut) {
            // This job needs to write data to the output file
            writing[j] = open(tmp->outFile, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        } else if (tmp->redirectOutAppend) {
            // This job needs to write data to the output file
            writing[j] = open(tmp->outFile, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);            
        }
        
        if (tmp->redirectIn) {
            // This job needs to read data from the inout file
            reading[j] = open(tmp->inFile, O_RDONLY); 
        }
        
        tmp = tmp->next; // Get next job
    }
    
    int i = 0;
    while (job != NULL) {
        
        if (strcmp(job->command, "exit") == 0) {
            for (p = 0; p < numcmds; p++)
            {
                if (reading[p] != -1) {
                    close(reading[p]);
                }
                
                if (writing[p] != -1) {
                    close(writing[p]);
                }
            }
            
            return -1;
        }
        
        // Parse command
        argsc = makeargs(job->command, &args);
        
        if (checkBuiltInCommands(job, args, argsc, addToHistory) == 1) {
            
            clean(argsc, args);
            
            job = job->next;
            
            continue;
        }
        
        if (addToHistory == 1) {
            addHistory(job);
        }
        
        // See if we can find an alias matching the command
        Node *alias1 = findAliasNode(args[0]);
        if (alias1 != NULL) {
            clean(argsc, args);
            argsc = makeargs(alias1->aliasCommand, &args);
        }
        
        pid_t childpid;
        int status;
        childpid=fork();
        
        if (childpid >= 0) 
        {
            if (childpid == 0) 
            {
                // This is the child process 
#if DEBUG
                // TESTING
                dprintf(newstdin, "Command '%s'\n", job->command);
                dprintf(newstdin, "writing[%d] = %d\n", i, writing[i]);
                dprintf(newstdin, "reading[%d] = %d\n", i, reading[i]);
#endif
                
                if (writing[i] != -1)
                {
                    close(STDOUT_FILENO);
                    dup2(writing[i],STDOUT_FILENO);
                }
                
                if (reading[i] != -1)
                {
                    close(0);
                    dup2(reading[i],0);
                }
                
                if (execvp(args[0], args) == -1) 
                {
                    perror("problem with command");
                    exit(0);
                }
            }
            else 
            {
                // parent
                wait(&status);
                
                clean(argsc, args);
                
                if (writing[i] != -1) {
                    close(writing[i]);
                }
                
                if (i > 0) 
                {
                    if (reading[i] != -1) {
                        close(reading[i]);
                    }
                }
            }
        }
        else
        {
            perror("fork");
        }
        
        i++;
        job = job->next;
    }
    
    return 1;
}
    
Job * createJob(char *command, char *inFile, char *outFile, int redirectOut, int redirectOutAppend, int redirectIn)
{
    Job *job = (Job *) malloc(sizeof(Job));
    job->isPiped = 0;
    job->redirectOut = redirectOut;
    job->redirectOutAppend = redirectOutAppend;
    job->redirectIn = redirectIn;
    job->alias = 0;
    
    job->prev = NULL;
    job->next = NULL;
    
    job->command = NULL;
    if (command != NULL) {
        job->command = malloc(sizeof(char *) * strlen(command));
        strcpy(job->command, command);
    }
    
    job->inFile = NULL;
    if (inFile != NULL) {
        job->inFile = malloc(sizeof(char *) * strlen(inFile));
        strcpy(job->inFile, inFile);
    }
    
    job->outFile = NULL;
    if (outFile != NULL) {
        job->outFile = malloc(sizeof(char *) * strlen(outFile));
        strcpy(job->outFile, outFile);
    }
    
    return job;
}

void freeJob(Job *job)
{
    free(job->command);
    job->command = NULL;
    
    if (job->inFile != NULL) {
        free(job->inFile);
        job->inFile = NULL;    
    }
    
    if (job->outFile != NULL) {
        free(job->outFile);
        job->outFile = NULL;    
    }
    
    free(job);
    job = NULL;

}

void cleanJobs(Job * job)
{
    Job * tmp;
    while (job != NULL) {
        tmp = job->next;
        freeJob(job);
        job = tmp;
    }
}

Job * getJobs(FILE * inputStream)
{    
    int j;
    int i = 0;
    char buf[MAX] = {};
    char inFilenameBuf[MAX] = {}; // This is the filename for input redirection
    char outFilenameBuf[MAX] = {}; // This is the filename for output redirection
    

    int inQuote = 0;
    char quoteChar;
    
    int redirectIn = 0;
    int redirectOut = 0;
    int redirectOutAppend = 0;
    
    Job *job = NULL;
    char c = getc(inputStream);
    while (1) {
        
    // Label so we can jump back to this switch statement after internal processing of more characters
    // this prevents us from having to pull another character off the buffer
    switchChar:
        
        switch (c)
        {                
            case ';':
                
                if (inQuote) {
                    buf[i] = c; // Add character to input
                    i++; // Increment pointer
                    c = getc(inputStream);
                    
                    continue;
                }
                
                buf[i] = '\0';
                if (job == NULL) {
                    job = createJob(buf, inFilenameBuf, outFilenameBuf, redirectOut, redirectOutAppend, redirectIn);
                } else {
                    job->next = createJob(buf, inFilenameBuf, outFilenameBuf, redirectOut, redirectOutAppend, redirectIn);
                    job->next->prev = job;
                    job = job->next;
                }
                
                redirectOut = redirectOutAppend = redirectIn = 0;
                
                clearBuffer(buf);
                clearBuffer(inFilenameBuf);
                clearBuffer(outFilenameBuf);
                i = 0;
            break;
                
            case '>':
                
                if (inQuote) {
                    buf[i] = c; // Add character to input
                    i++; // Increment pointer
                    c = getc(inputStream);
                    
                    continue;
                }
                
                c = getc(inputStream); // Get next character
                if (c == '>') { // Support >>
                    c = getc(inputStream);
                    redirectOutAppend = 1;
                } else {
                    redirectOut = 1;
                }
                
                j = 0;
                while (c != '\n' && c != ';' && c != '|' && c != '<') {
                    if (c != ' ') {
                        outFilenameBuf[j++] = c;
                    }
                    
                    c = getc(inputStream);
                }
                outFilenameBuf[j] = '\0';
                
                goto switchChar;                
            break;
                
            case '<':
                
                if (inQuote) {
                    buf[i] = c; // Add character to input
                    i++; // Increment pointer
                    c = getc(inputStream);

                    continue;
                }
                
                redirectIn = 1;
                    
                j = 0;
                c = getc(inputStream);
                while (c != '\n' && c != ';' && c != '|' && c != '>') {
                    if (c != ' ') {
                        inFilenameBuf[j++] = c;
                    }
                    
                    c = getc(inputStream);
                }
                inFilenameBuf[j] = '\0';
                
                goto switchChar;               
            break;
                
            case '|': // Our favorie, piping!
                
                if (inQuote) {
                    buf[i] = c; // Add character to input
                    i++; // Increment pointer
                    c = getc(inputStream);
                    
                    continue;
                }
                
                buf[i] = '\0';
                if (job == NULL) {
                    job = createJob(buf, inFilenameBuf, outFilenameBuf, redirectOut, redirectOutAppend, redirectIn);
                } else {
                    job->next = createJob(buf, inFilenameBuf, outFilenameBuf, redirectOut, redirectOutAppend, redirectIn);
                    job->next->prev = job;
                    job = job->next;
                }
                
                redirectOut = redirectOutAppend = redirectIn = 0;
                
                job->isPiped = 1;
                
                clearBuffer(inFilenameBuf);
                clearBuffer(outFilenameBuf);
                clearBuffer(buf);
                        
                i = 0;
            break;
                
            case '\n':
                if (buf[0] != '\0') { // Ensure we don't have an empty buffer
                    if (job == NULL) {
                        buf[i] = '\0';
                        job = createJob(buf, inFilenameBuf, outFilenameBuf, redirectOut, redirectOutAppend, redirectIn);
                    } else {
                        buf[i] = '\0'; // Add null terminator to buf 
                        job->next = createJob(buf, inFilenameBuf, outFilenameBuf, redirectOut, redirectOutAppend, redirectIn);
                        job->next->prev = job;
                        job = job->next;
                    }
                    
                    redirectOut = redirectOutAppend = redirectIn = 0;
                    
                    // Get the job pointer to the very first job
                    while (job->prev != NULL) {
                        job = job->prev;
                    }
                }
                
                clearBuffer(buf);
                clearBuffer(inFilenameBuf);
                clearBuffer(outFilenameBuf);
                i = 0;
                
                return job;
            break;
                
            case ' ':
                if (i != 0) { // Don't add space to first job
                    buf[i] = c; // Add character to input
                    i++; // Increment pointer
                }
            break;
                
            case '"': 
            case '\'':
                if (inQuote && quoteChar == c) {
                    inQuote = 0;
                } else {
                    inQuote = 1;
                }
                
                quoteChar = c;
                buf[i] = c; // Add character to input
                i++; // Increment pointer            
            break;
                
            default:
                buf[i] = c; // Add character to input
                i++; // Increment pointer
            break;
        }
        
        c = getc(inputStream);
    }
}

void readHistory()
{
    FILE *file = fopen(HISTFILE, "r");
    
    if (file == NULL) {
        return;
    }

    Job *job = getJobs(file);    
    while (job != NULL) {
        addHistory(job);
        
        job = job->next;
    }
}

void loadRc()
{
    FILE* file = fopen("./.mshrc", "r");

    if (file == NULL) {
        perror("opening file failed");
        
        return;
    }
    
    Job *job = getJobs(file);
    
    runJobs(job, 0);
    cleanJobs(job);
    fclose(file);
}

int main(int argc, const char * argv[])
{
    system("clear");
    
    loadRc();
    
    readHistory();
    
    Job * job = NULL;
    int run = 1;
    while (run == 1) {  
        
        printf("?:");
        job = getJobs(stdin);
        
        run = runJobs(job, 1);
        
        job = NULL;
    }
    
    cleanHistory();
    clearAliases();
    
    return 0;
}

