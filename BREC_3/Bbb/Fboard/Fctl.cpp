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

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"
#include "Fboard.h"

void usage( int exit_code )
{
    printf("-reset       pull program_b low and release\n");
    printf("-show        display state of done/init_b\n");
    printf("-rdid        issue spi sequence to read flash id\n");
    exit( exit_code );
}

int
main( int argc, char *argv[] )
{
    int            idx;
    int            val;
    char          *end;
    Fboard         fbrd;

    printf("Fctl: Enter Main\n");


    idx = 1;
    while( idx < argc ){

        if( 0==strcmp(argv[idx], "-help") ){
            usage(0);
        }

        else if( 0==strcmp(argv[idx], "-h") ){
            usage(0);
        }

        else if( 0==strcmp(argv[idx], "-reset") ){
           fbrd.Open();
           fbrd.Reset();
           fbrd.Close();
        }

        else if( 0==strcmp(argv[idx], "-show") ){
           fbrd.Open();
           fbrd.Show();
           fbrd.Close();
        }

        else if( 0==strcmp(argv[idx], "-rdid") ){
           unsigned char bf[256];

           bf[0] = 0x9F;
           bf[1] = 0x00;
           bf[2] = 0x00;
           bf[3] = 0x00;
           bf[4] = 0x00;

           fbrd.Open();
           fbrd.SpiXferStream8( bf, 5 );
           fbrd.Close();

           printf("bf[0]=0x%02x\n",bf[0]);
           printf("bf[1]=0x%02x\n",bf[1]);
           printf("bf[2]=0x%02x\n",bf[2]);
           printf("bf[3]=0x%02x\n",bf[3]);
           printf("bf[4]=0x%02x\n",bf[4]);
        }

        idx++;
    }


    printf("Fctl: Exit\n");
}
