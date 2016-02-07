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

#include "cli.h"

/**
 * Reset all parsing state of the provided cli object.
 */
void CliReset( Cli *cli )
{
    cli->prevCh = ' ';
    cli->curCh  = 0;
    cli->argc   = 0;
    cli->byteIdx= 0;
}

/**
 * Create a new cli object
 */
Cli *CliNew()
{
    Cli *cli;

    cli = (Cli*)malloc(sizeof(Cli));
    CliReset( cli );
    return( cli );
}

/*
 * Return a pointer to the specified argument index 
 */
char *CliArgByIndex( Cli *cli, int argc )
{
    int idx;
    int arg;
    int st;

    if( argc >= cli->argc ) return(NULL);

    idx = 0;
    st  = 0;
    arg = 0;
    while( idx < CLI_MAX_LINE_BYTES ){
        if( 0==cli->bytes[idx] ){
            if( arg == argc ) return( cli->bytes + st );
            arg++;
            st = idx + 1;
        }
        idx++;
    }
    return(NULL);
}

/**
 * Return a pointer to the string value of an arg (name specified)
 */
char *CliArgByNameAsStr( Cli *cli, const char *name )
{
    int idx;
    for(idx=0;idx<cli->argc;idx++){
       if( 0==strcmp(name,CliArgByIndex(cli,idx)) ){
           if( (idx+1)<cli->argc ){
               return( CliArgByIndex(cli,idx+1) );
           }
           else{
               return(NULL);
           }
       }
    }
    return(NULL);
}

/**
 * Return the integer value of an arg (name specified)
 */
int CliArgByNameAsNumber( Cli *cli, const char *name, double *nn )
{
    char *str;

    str = CliArgByNameAsStr(cli,name);
    if( !str ) return(-1);

    *nn = atof( str );
    return( 0 );
}

/**
 * Debug routine to display the parsing/input state of a cli object
 */
void CliShow( Cli *cli )
{
    int n;

    printf("prev '%c' 0x%02x\n",cli->prevCh, cli->prevCh);
    printf("cur  '%c' 0x%02x\n",cli->curCh , cli->curCh );
    printf("byteIdx   %d\n",cli->byteIdx);
    printf("argc      %d\n",cli->argc);
    for(n=0;n<cli->byteIdx;n++){
       printf("'%c' 0x%02x,  ",cli->bytes[n],cli->bytes[n]);
    }
    printf("\n");
    for(n=0;n<cli->argc;n++){
       printf("%d \"%s\"\n",n,CliArgByIndex( cli, n ) );  
    }
}

/** Internal method to identify white space */
static int CliIsWs( char ch )
{
   return( (' '==ch) || ('\t'==ch) || ('\r'==ch));
}

/** Internal method to identify escape character */
static int CliIsEsc( char ch )
{
   return( '\\'==ch );
}

/**
 * Inject an input character into a cli object. 
 * Returns:
 *     0 = no errors
 *    +1 = line read (and ready for use)
 *    <0 = error
 */
int CliInputCh( Cli *cli, char ch )
{
    // Make sure we still have room
    if( cli->byteIdx >= CLI_MAX_LINE_BYTES ) {
        return( -1 );
    }

    // Save current char, update previous
    cli->prevCh = cli->curCh;
    cli->curCh  = ch;

    // Escape next char
    if( CliIsEsc( cli->curCh) ){
        return( 0 );
    }  

    // End of line (eol)
    if( cli->curCh=='\n' ){

        // Escaped eol
        if( CliIsEsc( cli->prevCh ) ){
            return( 0 );
        }

        // Non Escaped eol
        else{

            // Non repetative non escaped whitespace
            if( !CliIsWs( cli->prevCh ) ){
               cli->bytes[ cli->byteIdx ] = 0;
               cli->byteIdx++;
               cli->argc++;
            } 
            return( 1 );
        }
    }

    // Whitespace
    if( CliIsWs( cli->curCh ) ){

        // Non Escaped whitespace
        if(  !CliIsEsc(cli->prevCh)  ) {

           // Non repetative non escaped whitespace
           if( !CliIsWs( cli->prevCh ) ){
               cli->bytes[ cli->byteIdx ] = 0;
               cli->byteIdx++;
               cli->argc++;
           } 
           return( 0 );
        }

        // Escaped whitespace
        else{
           ; // escaped white space - fall through
        }
    }

    // Add character to token list
    cli->bytes[ cli->byteIdx ] = ch;
    cli->byteIdx++;
    return( 0 );
}
