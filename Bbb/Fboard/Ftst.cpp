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

void usage( int exit_code )
{
    printf("-usleep <M>  sleeps script execution for <M> micro seconds\n");
    printf("-write  <N>  write the 16 bit word N\n");
    printf("-echo   <str> echo string <str>\n");
    printf("\n");
    printf("e.g. -write 0x00001\n");
    exit( exit_code );
}

int      usHold  = 1000;
int      fspiDbg = 0;

GpioUtil hss;
GpioUtil hsclk;
GpioUtil hmosi[2];
GpioUtil hmiso[2];

int
fspi_init( )
{
    hss.Define(      117 );
    hsclk.Define(    110 );
    hmosi[0].Define( 113 );
    hmosi[1].Define( 115 );
    hmiso[0].Define( 111 );
    hmiso[1].Define( 112 );

    hss.Export();
    hsclk.Export();
    hmosi[0].Export();
    hmosi[1].Export();
    hmiso[0].Export();
    hmiso[1].Export();

    hss.SetDirInput(      0 );
    hsclk.SetDirInput(    0 );
    hmosi[0].SetDirInput( 0 );
    hmosi[1].SetDirInput( 0 );
    hmiso[0].SetDirInput( 1 );
    hmiso[1].SetDirInput( 1 );

    hss.Open();
    hsclk.Open();
    hmosi[0].Open();
    hmosi[1].Open();
    hmiso[0].Open();
    hmiso[1].Open();

    hss.Set(1);
    hsclk.Set(0);

    return(0);
}

int 
fspi_write( int wval )
{
    int rval;
    int obit,ibit,idx;

    if( fspiDbg ){
       printf("fspi_write: wval = 0x%04x\n",wval);
    }

    rval    = 0;

    // expecting: sclk=0, ss=1
    hss.Set( 0 );
    us_sleep( usHold );

    for( idx=15; idx>=0; idx-- ){
        if( fspiDbg ){
            printf("idx[%d]\n",idx);
        }

        if( wval&0x8000 ) obit = 1;
        else              obit = 0;

        if( fspiDbg ){
            printf("   obit = %d\n",obit);
        }
        hmosi[0].Set( obit );
        us_sleep( usHold );
        hsclk.Set( 1 );
        us_sleep( usHold );
        ibit = hmiso[0].Get( );
        if( fspiDbg ){
            printf("   ibit = %d\n",ibit);
        }

        us_sleep( usHold );
        hsclk.Set( 0 );
        us_sleep( usHold );

        rval = (rval<<1) | ibit;
        wval = (wval<<1);
    }

    // expecting: sclk=0, ss=0

    hss.Set( 1 );
    us_sleep( usHold );

    if( fspiDbg ){
        printf("fspi_write: rval = 0x%04x\n",rval);
    }
    return(rval);
}

int
main( int argc, char *argv[] )
{
    int            idx;
    int            val;
    char          *end;
    unsigned short rd;

    printf("Ftst: Enter Main\n");

    fspi_init( );
 
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

        else if( 0==strcmp(argv[idx], "-write") ){

            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);
            
            rd = fspi_write( val );
            printf("w=0x%04hx r=0x%04hx (%hd)\n",val, rd, rd);

            idx+=2;
            continue;
        }

        idx++;
    }

    printf("Ftst: Enter Exit\n");
}

