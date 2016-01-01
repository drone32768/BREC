//
//
// This source code is available under the "Simplified BSD license".
//
// Copyright (c) 2016, J. Kleiner
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, 
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright 
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the original author nor the names of its contributors 
//    may be used to endorse or promote products derived from this software 
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
// OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


FILE *inputFh;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include "../Util/mcf.h"
#include "../Util/gpioutil.h"
#include "../JtagTools/xsvfLib.h"
#include "../JtagTools/jtag_bs.h"

unsigned char xsvf_next_byte()
{
   return( (unsigned char)fgetc(inputFh) );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] )
{
    int     err   = 0;
    char*   fName = 0;
    int     i;

    for ( i = 1; i < argc ; ++i )
    {
        if ( 0==strcmp( argv[ i ], "-v" ) )
        {
            ++i;
            if ( i >= argc )
            {
                printf( "ERROR:  missing <level> parameter for -v option.\n" );
            }
            else
            {
                xsvf_iDebugLevel    = atoi( argv[ i ] );
                printf( "Verbose level = %d\n", xsvf_iDebugLevel );
            }
        }
        else
        {
            fName  = argv[ i ];
        }
    }

    if ( !fName )
    {
        printf( "usage [-v level] file\n" );
        printf( " -v level   verbose, level = 0-4 (default=0)\n" );
        printf( " file       xsvf file to execute\n" );
        return( -254 );
    }
    printf( "Loading file = %s\n", fName );

    jtag_bs_open();
    jtag_bs_set_tms(1);

    inputFh = fopen( fName, "rb" );
    if ( !inputFh )
    {
        fprintf(stderr,"ERROR:  Cannot open file %s\n", fName );
        return( -255 );
    }
    err  = xsvfExecute();
    fclose( inputFh );

    printf( "Done w/ err = %d\n", err );

    return( err );
}
