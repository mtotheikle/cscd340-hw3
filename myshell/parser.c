//
//  parser.c
//  myshell
//
//  Created by Michael Williams on 10/15/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include "parser.h"
#include "declarations.h"
#include "utilities.h"

// The current buffer we are working with
char buf[MAX];

// This is the filename for input redirection
char inFilenameBuf[MAX]; 

// This is the filename for output redirection
char outFilenameBuf[MAX]; 

// Flag for if we are inside of a quote right now or note
int inQuote = 0;

// Character holder so we know what quote character we are currently in
char quoteChar;

// Flags for redirection
int redirectIn = 0;
int redirectOut = 0;
int redirectOutAppend = 0;

void resetParser()
{
    clearBuffer(buf);
    clearBuffer(inFilenameBuf);
    clearBuffer(outFilenameBuf);
    inQuote = 0;
    quoteChar = '\0';
    redirectIn = 0;
    redirectOut = 0;
    redirectOutAppend = 0;
}