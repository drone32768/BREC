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
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "SiEstimate.h"
#include "../Util/mcf.h"

/**
 * This is a standalone test program to test and benchmark the performance
 * of the SiEstimate class.
 *
 */

////////////////////////////////////////////////////////////////////////////////
static void test_signal( int pattern, short *bf, int npts )
{
    int k,n;

    // fill input with test signal
    k = pattern;
    for( n=0; n<npts; n++ ){
        bf[ n ] = 32767*sin( k*n*2.0*M_PI / npts );
    }
}

////////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] )
{
    SiEstimator m2;
    int         idx;
    int         npts;
    int         fast,table;
    short *samples;
    StopWatch   sw;
    int         m;

    // defaults
    npts  = 16;
    fast  = 0;

    // parse command line args
    idx   = 1;
    while( idx < argc ){
        if( 0==strcmp("-f",argv[idx]) ){
            fast = 1;
        }
        if( 0==strcmp("-n",argv[idx]) ){
            idx++;
            npts = atoi( argv[idx] );
        }
        if( 0==strcmp("-t",argv[idx]) ){
            table = 1;
        }
        idx++;
    }

    // setup input samples
    samples = (short*)malloc( sizeof(short)*npts );
    test_signal( 1, samples, npts );

    // non table test
    if( !table ){
        printf("fast = %d\n",fast);
        printf("pts  = %d\n",npts);
        m2.Configure( npts, 0, fast, npts/2, 1 );
        sw.Start();
        m2.Pse( samples,npts );
        sw.Stop();
        printf("us   = %lu\n", sw.GetuS() );
    }

    //  table performance summary
    if( table ){
        printf(" f,       N,  uS\n");
        for( m=0;m<2; m++){
            npts = 256;
            while( npts<32768 ){
                m2.Configure( npts, 0, m, npts/2, 1 );

                sw.Start();
                m2.Pse( samples, npts );
                sw.Stop();
                printf("%2d, %8d, %lu\n",m,npts,sw.GetuS());
                npts = npts * 2;
            }
        }
    }
}

