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
/*
 * This is a simple stand alone utility to exercize an F board host SPI 
 * interface. Layered scripts use this utility to test the board and fpga image
 *
 * All spi communication is done using the gpio from the cpu and
 * contained within this file.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"
#include "Fboard.h"

void usage( int exit_code )
{
    printf("-usleep <M>  sleeps script execution for <M> micro seconds\n");
    printf("-write  <N>  write the 16 bit word N\n");
    printf("-echo   <str> echo string <str>\n");
    printf("\n");
    printf("e.g. -write 0x00001\n");
    exit( exit_code );
}

int
main( int argc, char *argv[] )
{
    int            idx;
    int            val;
    char          *end;
    unsigned short rd;
    Fboard         fbrd;

    printf("Ftst: Enter Main\n");

    fbrd.Open();
    fbrd.Show();
 
    idx = 1;
    while( idx < argc ){

        if( 0==strcmp(argv[idx], "-help") ){
            usage(0);
        }

        else if( 0==strcmp(argv[idx], "-h") ){
            usage(0);
        }

        else if( 0==strcmp(argv[idx], "-usleep") ){
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);
            us_sleep( val );
        } 

        else if( 0==strcmp(argv[idx], "-echo") ){
            if( (idx+1) >= argc ){ usage(-1); }
            printf("%s\n",argv[idx+1]);
        } 

        else if( 0==strcmp(argv[idx], "-show") ){
            fbrd.Show();
        }

        else if( 0==strcmp(argv[idx], "-write") ){
            unsigned char bf[256];

            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);
            
            bf[0] = (val>>8)&0xff;
            bf[1] = (val   )&0xff;
            fbrd.SpiXfer( bf, 2 );
            rd    = bf[0];
            rd    = (rd<<8 ) + bf[1];

            printf("w=0x%04hx r=0x%04hx (%hd)\n",val, rd, rd);

            idx+=2;
            continue;
        }

        idx++;
    }

    printf("Ftst: Enter Exit\n");
}

