//
//  utilities.h
//  myshell
//
//  Created by Michael Williams on 10/15/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef myshell_utilities_h
#define myshell_utilities_h

void strip(char * s);

void clean(int argc, char **argv);

int makeargs(char *s, char *** argv);

void clearBuffer(char *buf);

void cleanJobs(Job * job);

#endif
