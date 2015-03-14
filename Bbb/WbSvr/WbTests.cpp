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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

#include "WbTests.h"

//------------------------------------------------------------------------------
/**
Filter response scanning builtin test reference diagram.

          LO0                     LO1
            |                      |
            |                      |
            V                      V

    B board                 C board                 A board
    +-----------+          +-----------+           +---------------+
    |      Aux  |          |       Aux |           |               |
    |           |          |           |           |               |
    |     IFout |-----     |     IFout |---------->| IFin     Net  |
    |           |     |    |           |           |               |
    |      RFin |     ---->|      RFin |           |               |
    |           |          |           |           |               |
    +-----------+          +-----------+           +---------------+
                       ^                        ^                          
                       |                        |                          
                     IF1=1575.420 MHz      IF2=10.700 MHz

This method conducts a sweep of LO0 (B board) across the first SAW filter
range.  The LO1 (C board) is maintained 10.7MHz above LO0 resulting in 
a C board IF output at an A board input frequency.  At each step
the AC rms power is collected along with the DC bias for testing.  All
results are stored in a .csv file /tmp.  Several difference sweeps
can be effected in step and span.  Sweep numbers range from [0..2]
                                           
Returns 0 on successful sweep, non zero on error.
*/

int Test00( Adf4351 *syn0, Adf4351 *syn1, AdcIf *adc, int sweep )
{
    FILE   *fp;
    int    ns  = 32768;
    double if1 = 1575420000;
    double if2 = 10750000;
    double l0,l1;
    double rms,mean;
    double srcHz,dHz,startHz, stopHz;
    char   lineBf[256];
    int    err,loErr;
    short  *sampleBf;


    // Configure sweep 
    if( 0==sweep ){
       startHz = if1 - 170000000;
       stopHz  = if1 + 170000000;
       dHz     = 10000000;
    }
    if( 1==sweep ){
       startHz = if1 - 170000000;
       stopHz  = if1 + 170000000;
       dHz     = 1000000;
    }
    if( 2==sweep ){
       startHz = if1 - 300000000; 
       stopHz  = if1 + 300000000; 
       dHz     = 10000000;
    }

    // Open output file
    sprintf(lineBf,"/tmp/saw-response-sn-%d.csv",sweep);
    fp = fopen(lineBf,"w");
    if( !fp ){
        perror("fopen on f0 csv output");
        return(-1);
    }

    sampleBf = (short*)malloc( ns*sizeof(short) );

    // Begin sweep
    srcHz   = startHz;
    loErr   = 0;
    fprintf(fp, "Hz,rmsDB,mean,nSample,LO0,LO1,loerr\n");
    while( srcHz<=stopHz ){
        l0 = srcHz;
	l1 = srcHz + if2;

        err = syn0->SetFrequencyWithLock( l0, 100, 1000000 );
        if( 0==err ) loErr++;

        err = syn1->SetFrequencyWithLock( l1, 100, 1000000 );
        if( 0==err ) loErr++;

        us_sleep( 100000 );
        mean = adc->GetRms( ns, sampleBf, &rms);
        fprintf(fp,"%f, %f, %f, %f, %f, %f, %d\n",
                srcHz,rms,mean,(float)ns,l0,l1,loErr);
        printf (   "%f, %f, %f, %f, %f, %f, %d\n",
                srcHz,rms,mean,(float)ns,l0,l1,loErr);
        srcHz += dHz;
    }

    // Close file and done
    fclose(fp);
    free(sampleBf);

    return(0);
}

//------------------------------------------------------------------------------
int Test01( Adf4351 *syn0, Adf4351 *syn1, AdcIf *adc, int sweep )
{
    FILE   *fp;
    int    ns  = 65536; 
    double if1 = 1530000000; // pull down from first filter max to reduce level
    double if2 = 10750000;
    double l0,l1;
    double rms,mean;
    double srcHz,dHz,startHz, stopHz;
    char   lineBf[256];
    int    err,loErr;
    short  *sampleBf;


    // Configure sweep 
    if( 0==sweep ){ // med, quick scan 9.25 -> 12.15 MHz
       startHz = if2 - 1500000;
       stopHz  = if2 + 1500000;
       dHz     = 50000;
    }
    if( 1==sweep ){ // med, fine, same as previous but more detail
       startHz = if2 - 1500000;
       stopHz  = if2 + 1500000;
       dHz     = 5000;
    }
    if( 2==sweep ){ // wide scan 5.75 -> 15.75 MHz
       startHz = if2 - 5000000; 
       stopHz  = if2 + 5000000; 
       dHz     = 50000;
    }
    if( 3==sweep ){ // narrow, fine,  band scan 
       startHz = 10000000;
       stopHz  = 11500000;
       dHz     = 5000;
    }
    if( 4==sweep ){ // wide scan 1.00 -> 41.00 MHz
       startHz = 1000000;
       stopHz  = 40000000;
       dHz     = 100000;
    }

    // Open output file
    sprintf(lineBf,"/tmp/a-response-sn-%d.csv",sweep);
    fp = fopen(lineBf,"w");
    if( !fp ){
        perror("fopen on adc csv output");
        return(-1);
    }

    sampleBf = (short*)malloc( ns*sizeof(short) );

    // Begin sweep
    srcHz   = startHz;
    loErr   = 0;
    l0      = if1;
    err = syn0->SetFrequencyWithLock( l0, 100, 1000000 );
    if( 0==err ) loErr++;
    fprintf(fp, "Hz,rmsDB,mean,nSample,LO0,LO1,loerr\n");
    while( srcHz<=stopHz ){
	l1 = l0 + srcHz;

        err = syn1->SetFrequencyWithLock( l1, 100, 1000000 );
        if( 0==err ) loErr++;

        us_sleep( 100000 );
        mean = adc->GetRms( ns, sampleBf, &rms);
        fprintf(fp,"%f, %f, %f, %f, %f, %f, %d\n",
                srcHz,rms,mean,(float)ns,l0,l1,loErr);
        printf (   "%f, %f, %f, %f, %f, %f, %d\n",
                srcHz,rms,mean,(float)ns,l0,l1,loErr);
        srcHz += dHz;
    }

    // Close file and done
    fclose(fp);
    free(sampleBf);

    return(0);
}
