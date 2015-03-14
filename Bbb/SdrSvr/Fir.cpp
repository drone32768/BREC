//
//
// This source code is available under the "Simplified BSD license".
//
// Copyright (c) 2013, J. Kleiner
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
///
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Fir.h"

#define FILTER5

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#ifdef FILTER0  
/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 1000000 Hz

fixed point precision: 16 bits

* 0 Hz - 400000 Hz
  gain = 1
  desired ripple = 0.1 dB
  actual ripple = n/a

* 450000 Hz - 500000 Hz
  gain = 0
  desired attenuation = -80 dB
  actual attenuation = n/a

*/

#define FILTER_TAP_NUM 71

static int filter_taps[FILTER_TAP_NUM] = {
  72,
  97,
  -70,
  47,
  -10,
  -42,
  100,
  -145,
  155,
  -114,
  19,
  117,
  -259,
  363,
  -383,
  287,
  -72,
  -230,
  548,
  -789,
  861,
  -693,
  270,
  355,
  -1058,
  1658,
  -1953,
  1760,
  -962,
  -454,
  2382,
  -4604,
  6823,
  -8714,
  9983,
  55105,
  9983,
  -8714,
  6823,
  -4604,
  2382,
  -454,
  -962,
  1760,
  -1953,
  1658,
  -1058,
  355,
  270,
  -693,
  861,
  -789,
  548,
  -230,
  -72,
  287,
  -383,
  363,
  -259,
  117,
  19,
  -114,
  155,
  -145,
  100,
  -42,
  -10,
  47,
  -70,
  97,
  72
};
#endif

#ifdef FILTER1  
/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 1250000 Hz

fixed point precision: 16 bits

* 0 Hz - 250000 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = n/a

* 350000 Hz - 625000 Hz
  gain = 0
  desired attenuation = -80 dB
  actual attenuation = n/a

*/

#define FILTER_TAP_NUM 27

static int filter_taps[FILTER_TAP_NUM] = {
  115,
  388,
  288,
  -1081,
  -3360,
  -4010,
  -1062,
  2833,
  2111,
  -3566,
  -5668,
  3901,
  20395,
  28704,
  20395,
  3901,
  -5668,
  -3566,
  2111,
  2833,
  -1062,
  -4010,
  -3360,
  -1081,
  288,
  388,
  115
};
#endif

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#ifdef FILTER2  
/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 1250000 Hz

fixed point precision: 16 bits

* 0 Hz - 250000 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = n/a

* 335000 Hz - 625000 Hz
  gain = 0
  desired attenuation = -35 dB
  actual attenuation = n/a

*/

#define FILTER_TAP_NUM 15

static int filter_taps[FILTER_TAP_NUM] = {
  1803,
  4869,
  3342,
  -2773,
  -6100,
  3189,
  20482,
  29509,
  20482,
  3189,
  -6100,
  -2773,
  3342,
  4869,
  1803
};
#endif

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#ifdef FILTER3  
/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 1250000 Hz

fixed point precision: 16 bits

* 0 Hz - 250000 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = n/a

* 335000 Hz - 625000 Hz
  gain = 0
  desired attenuation = -20 dB
  actual attenuation = n/a

*/

#define FILTER_TAP_NUM 9

static int filter_taps[FILTER_TAP_NUM] = {
  -4296,
  -9126,
  3090,
  20101,
  30805,
  20101,
  3090,
  -9126,
  -4296
};
#endif

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#ifdef FILTER4  
/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 1000000 Hz

fixed point precision: 16 bits

* 0 Hz - 400000 Hz
  gain = 1
  desired ripple = 0.1 dB
  actual ripple = n/a

* 450000 Hz - 500000 Hz
  gain = 0
  desired attenuation = -60 dB
  actual attenuation = n/a

*/

#define FILTER_TAP_NUM 57

static int filter_taps[FILTER_TAP_NUM] = {
  -172,
  57,
  -18,
  -56,
  148,
  -234,
  278,
  -249,
  129,
  75,
  -327,
  567,
  -715,
  700,
  -474,
  37,
  550,
  -1167,
  1650,
  -1822,
  1528,
  -676,
  -732,
  2590,
  -4694,
  6772,
  -8529,
  9705,
  55418,
  9705,
  -8529,
  6772,
  -4694,
  2590,
  -732,
  -676,
  1528,
  -1822,
  1650,
  -1167,
  550,
  37,
  -474,
  700,
  -715,
  567,
  -327,
  75,
  129,
  -249,
  278,
  -234,
  148,
  -56,
  -18,
  57,
  -172
};
#endif

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#ifdef FILTER5  
/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 1000000 Hz

fixed point precision: 16 bits

* 0 Hz - 350000 Hz
  gain = 1
  desired ripple = 0.1 dB
  actual ripple = n/a

* 450000 Hz - 500000 Hz
  gain = 0
  desired attenuation = -60 dB
  actual attenuation = n/a

*/

#define FILTER_TAP_NUM 31

static int filter_taps[FILTER_TAP_NUM] = {
  -145,
  -12,
  193,
  -414,
  504,
  -272,
  -359,
  1229,
  -1913,
  1831,
  -496,
  -2220,
  5923,
  -9747,
  12631,
  51832,
  12631,
  -9747,
  5923,
  -2220,
  -496,
  1831,
  -1913,
  1229,
  -359,
  -272,
  504,
  -414,
  193,
  -12,
  -145
};
#endif

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

Fir::Fir()
{
    mTrace = 0;
    mIdx   = 0;
}

void Fir::SetTrace( int tr )
{
    mTrace = tr;
}

short Fir::Filter( int eo, short x )
{
    int sum;
    int    n,tn;

    mSamples[ mIdx ] = x;
    n   = mIdx; 
    mIdx=(mIdx+1)%FILTER_TAP_NUM;   
    sum = 0;

    for(tn=eo;tn<FILTER_TAP_NUM;tn+=2){
        sum += filter_taps[tn] * mSamples[n];
        n = (n+FILTER_TAP_NUM-1)%FILTER_TAP_NUM;
    }
    sum = sum >> 16;

    if( mTrace ) printf("%d , %d\n",x,(short)sum);

    return( (short)sum );
}

