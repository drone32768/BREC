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
//
#ifndef __H_BOARD__
#define __H_BOARD__

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"
#include "../Interfaces/AdcIf.h"

class Hboard : public AdcIf {
    unsigned int            mPidx;
    unsigned int            mDecimate;
    unsigned int            mSimImage;
    volatile unsigned short *mPtrPruSamples;
    volatile unsigned char *mPtrPruSram;
    volatile unsigned int  *mPtrHead;
    int                     mCSPS;
    GpioUtil                mGpioEnable;

    int                     mSamplingType;

#   define SAMPLE_TYPE_DOWNSAMPLE 1
#   define SAMPLE_TYPE_CIC_2_2    2
#   define SAMPLE_TYPE_POLY_FIR   3


public:
    Hboard();
    Gpio6PinGroup* AllocPort( int pn );
    int Open();
    int Flush();
    int FlushSamples();
    int GetSamplePair( short *eo ); 

    int Get2kSamples( short *bf ); 
    int Get2kSamples_DownSample( short *bf ); 
    int Get2kSamples_Cic_2_2( short *bf ); 
    int Get2kSamples_PolyFir( short *bf ); 

    int SetComplexSampleRate( int complexSamplesPerSecond );
    int GetComplexSampleRate();
    int StartPrus();
    int GetRms( int nSamples, short *aSamples, double *rrms );
    int SetGain( int gn );

    int SetSim( int sim );
};

#endif
