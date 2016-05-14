//
// This source code is available under the "Simplified BSD license".
//
// Copyright (c) 2015, J. Kleiner
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
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <string>
#include <iostream>

#include "Device.h"
#include "DevSim.h"

//------------------------------------------------------------------------------
DevSim::DevSim()
{
}

//------------------------------------------------------------------------------
int DevSim::Open()
{
    printf("*********** DevSim::Open Begin ***************************\n");
    printf("*********** DevSim::Open End *****************************\n");
    return(0);
}

//------------------------------------------------------------------------------
int DevSim::GetComplexSampleRate()
{
    return(0);
}

//------------------------------------------------------------------------------
int DevSim::FlushSamples()
{
    return(0);
}

//------------------------------------------------------------------------------
int    gChId   = 0;
double gFsHz   = 1;
double gTuneHz = 1;
double gToneHz = 750.1e6;
short  gToneAmp= 1024;

//------------------------------------------------------------------------------
int DevSim::Get2kSamples( short *dst )
{
    int    idx,n0;
    double dfHz,phi;

    // Start with noise in all cases
    for( idx=0; idx<2048; idx++ ){
        dst[idx] = random() % 256;
    }

    dfHz = gTuneHz - gToneHz;

    // If the test tone is outside of the channel bw then done
    if( fabs(dfHz) > (gFsHz/2) ){
        return(0);
    }

printf("gFsHz=%f, gTuneHz=%f, gToneHz=%f\n",gFsHz,gTuneHz,gToneHz);

    // Add the test tone
    for( idx=0, n0=0; idx<1024; idx+=2, n0++ ){
        phi = 2.0 * M_PI * n0 * dfHz / gFsHz;
        dst[idx]   += (short)( gToneAmp * sin( phi ) );
        dst[idx+1] += (short)( gToneAmp * cos( phi ) );
    }

    return(0);
}

//------------------------------------------------------------------------------
int DevSim::SetChannel( int chId )
{
    printf("%s:%d: Chnl %d\n",__FILE__,__LINE__,chId);

    gChId = chId;
    switch( chId ){
        case 5:{
            gFsHz = 0.2e6;
            break;
        }
        case 4:{
            gFsHz = 4.0e6;
            break;
        }
        case 3:
        default :{
            gFsHz = 40.0e6;
            break;
        }
    }

    return(0);
}

//------------------------------------------------------------------------------
double DevSim::SetTuneHz( double freqHz )
{
    // printf("%s:%d: Tune %f MHz\n",__FILE__,__LINE__,freqHz/1e6);
    us_sleep( 10000 );
    gTuneHz = freqHz;
    return(freqHz);
}
