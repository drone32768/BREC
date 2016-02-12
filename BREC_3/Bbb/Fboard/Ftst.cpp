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
#include <arpa/inet.h>
#include <sys/time.h>

#include "Util/mcf.h"
#include "Util/gpioutil.h"
#include "Fboard.h"

Fboard         fbrd;

// Pattern tests expect BDC based fpga image
#define BDC_READ_R3 0x8300

void usage( int exit_code )
{
    printf("-usleep <M> sleeps script execution for <M> micro seconds\n");
    printf("-echo   <S> echo string <S>\n");
    printf("-write  <N> write the 16 bit word N\n");
    printf("-w16x2  <N> write the 16 bit word N using 2x SPI\n");
    printf("-show       show state of sw and device\n");
    printf("-pat01      couting pattern test w/   1 x  8 bit stream xfer\n");
    printf("-pat02      couting pattern test w/ 512 x 16 bit array xfer\n");
    printf("-pat03      couting pattern test w/ 512 x 16 bit array xfer 2x\n");
    printf("\n");
    printf("e.g. -write 0x00001\n");
    exit( exit_code );
}

void pat01()
{
    int            idx;
    unsigned short sbf[128];
    unsigned short vAct,vExp;
    unsigned int   totCnt,errCnt,intCnt;
    int            us;
    struct timeval tv0, tv1;

    printf("NOTE: This test does not self terminate.\n");

    totCnt     = 0;
    errCnt     = 0;
    intCnt     = 0;
    vExp       = 0;
    gettimeofday( &tv0, NULL );
    while( 1 ){

        sbf[0] = ntohs( BDC_READ_R3 );
        fbrd.SpiXferStream8( (unsigned char*)sbf, 2 );

        vAct = htons( sbf[0] );

        if( vAct!=vExp ){
                   printf("    ERR read 0x%02x (%d), expect 0x%02x (%d) @%d\n",
                         vAct,
                         vAct,
                         vExp,
                         vExp,
                         totCnt
                     );
               if( errCnt > 30 ) exit( -1 );
               errCnt++;
               vExp = vAct;
        }
        totCnt++;
        intCnt++;
        vExp++;

        gettimeofday( &tv1, NULL );
        if( (tv1.tv_sec-tv0.tv_sec) > 3 ){
           us = tv_delta_useconds( &tv1, &tv0 );
           printf("Total=%d, err=%d, M8rw/sec=%f\n",
                      totCnt,
                      errCnt,
                      (double)intCnt/(double)us
                 );
           tv0 = tv1;
           intCnt=0;
        }

    } // end of infinite loop

}

void pat02()
{
    int            idx;
    unsigned short sbf[1024];
    unsigned short vAct,vExp;
    unsigned int   totCnt,errCnt,intCnt,xferCnt;
    int            us;
    struct timeval tv0, tv1;

    printf("NOTE: This test does not self terminate.\n");

    totCnt     = 0;
    errCnt     = 0;
    intCnt     = 0;
    vExp       = 0;
    xferCnt    = 512;
    gettimeofday( &tv0, NULL );
    while( 1 ){

        for(idx=0;idx<xferCnt;idx++){
            sbf[idx] = BDC_READ_R3;
        }

        fbrd.SpiXferArray16( sbf, xferCnt );

        for(idx=0;idx<xferCnt;idx++){
            vAct = sbf[idx];
            if( vAct!=vExp ){
                   printf("    ERR read 0x%02x (%d), expect 0x%02x (%d) @%d\n",
                         vAct,
                         vAct,
                         vExp,
                         vExp,
                         totCnt
                     );
               if( errCnt > 30 ) return;
               errCnt++;
               vExp = vAct;
            }
            totCnt++;
            intCnt++;
            vExp++;
        }

        gettimeofday( &tv1, NULL );
        if( (tv1.tv_sec-tv0.tv_sec) > 3 ){
           us = tv_delta_useconds( &tv1, &tv0 );
           printf("Total=%d, err=%d, M16rw/sec=%f\n",
                      totCnt,
                      errCnt,
                      (double)intCnt/(double)us
                 );
           tv0 = tv1;
           intCnt=0;
        }

    } // end of infinite loop

}

void pat03()
{
    int            idx;
    unsigned short sbf[1024];
    unsigned short vAct,vExp;
    unsigned int   totCnt,errCnt,intCnt,xferCnt;
    int            us;
    struct timeval tv0, tv1;

    printf("NOTE: This test does not self terminate.\n");

    totCnt     = 0;
    errCnt     = 0;
    intCnt     = 0;
    vExp       = 0;
    xferCnt    = 512;
    gettimeofday( &tv0, NULL );
    while( 1 ){

        for(idx=0;idx<xferCnt;idx++){
            sbf[idx] = BDC_READ_R3; 
        }

        fbrd.SpiXferArray16x2( sbf, xferCnt );

        for(idx=0;idx<xferCnt;idx++){
            vAct = sbf[idx];
            if( vAct!=vExp ){
                   printf("    ERR read 0x%02x (%d), expect 0x%02x (%d) @%d\n",
                         vAct,
                         vAct,
                         vExp,
                         vExp,
                         totCnt
                     );
               if( errCnt > 30 ) return;
               errCnt++;
               vExp = vAct;
            }
            totCnt++;
            intCnt++;
            vExp++;
        }

        gettimeofday( &tv1, NULL );
        if( (tv1.tv_sec-tv0.tv_sec) > 3 ){
           us = tv_delta_useconds( &tv1, &tv0 );
           printf("Total=%d, err=%d, (2x)M16rw/sec=%f\n",
                      totCnt,
                      errCnt,
                      (double)intCnt/(double)us
                 );
           tv0 = tv1;
           intCnt=0;
        }

    } // end of infinite loop

}

int
main( int argc, char *argv[] )
{
    int            idx;
    int            val;
    char          *end;
    unsigned short rd;

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

        else if( 0==strcmp(argv[idx], "-pat01") ){
            pat01();
        }

        else if( 0==strcmp(argv[idx], "-pat02") ){
            pat02();
        }

        else if( 0==strcmp(argv[idx], "-pat03") ){
            pat03();
        }

        // TODO this is really w16x1
        else if( 0==strcmp(argv[idx], "-write") ){
            unsigned char bf[256];

            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);
            
            bf[0] = (val>>8)&0xff;
            bf[1] = (val   )&0xff;
            fbrd.SpiXferStream8( bf, 2 );
            rd    = bf[0];
            rd    = (rd<<8 ) + bf[1];

            printf("w=0x%04hx r=0x%04hx (%hd)\n",val, rd, rd);

            idx+=2;
            continue;
        }

        else if( 0==strcmp(argv[idx], "-w16x2") ){
            unsigned short sbf[128];

            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);
            
            sbf[0] = val;
            fbrd.SpiXferArray16x2( sbf, 1 );
            rd    = sbf[0];

            printf("2x: w=0x%04hx r=0x%04hx (%hd)\n",val, rd, rd);

            idx+=2;
            continue;
        }
        idx++;
    }

    printf("Ftst: Enter Exit\n");
}

