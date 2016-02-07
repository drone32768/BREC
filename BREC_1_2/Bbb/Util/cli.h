/*
 *
 * This source code is available under the "Simplified BSD license".
 *
 * Copyright (c) 2013, J. Kleiner
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the original author nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef __CLI_H__
#define __CLI_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/**
 * For simplicity and performance the cli line size and argument
 * count are fixed.  This is really for micro implementations
 * where only a single cli instances is needed and can be used.
 */
#define CLI_MAX_LINE_BYTES 256
#define CLI_MAX_ARGS       64

/**
 * This object contains the state of a cli parsing session.
 * Characters are input to such an object, parsed and then fetched
 * by index and name.  The implementation is C to support small embedded
 * applications (e.g. 8 bit micro controllers with 10's of kB).
 */
typedef struct {

    /** last character input */
    char  prevCh;

    /** current character input */
    char  curCh;

    /** number of arguments parsed */
    int   argc;

    /** index into line buffer of next input character */
    int   byteIdx;

    /** line buffer */
    char  bytes[CLI_MAX_LINE_BYTES];

} Cli;

void  CliReset( Cli *cli );
Cli  *CliNew();
char *CliArgByIndex( Cli *cli, int argc );
char *CliArgByNameAsStr( Cli *cli, const char *name );
int   CliArgByNameAsNumber( Cli *cli, const char *name, double *nn );
void  CliShow( Cli *cli );
int   CliInputCh( Cli *cli, char ch );

#endif
