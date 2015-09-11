//
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
#ifndef __X_BOARD__
#define __X_BOARD__

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"
#include "../Interfaces/AdcIf.h"
#include "../Interfaces/MixerIf.h"
#include "../Iboard/Iboard.h"

class Xboard : public AdcIf, public MixerIf {

private:
    int                      mXspiDbg;
    int                      mCSPS;    

    ////////////////////////////////////////
    int                      mPidx;
    volatile unsigned short *mPtrPruSamples;
    volatile unsigned char  *mPtrPruSram;

    // gpio to enable i board port 2
    GpioUtil                 mPortEnable;

    // Output formating parameters
    int                      mOutFmtShift;
    int                      mOutFmtAdd;

    // Other internal controls
    int                      mFifoSrc;
    int                      mTpg;

    void SetR3(); 

public:
    Xboard();

    // ADC interfaces
    int Open();
    int Flush();
    int FlushSamples();
    int GetSamplePair( short *eo ); 
    int Get2kSamples( short *bf ); 
    int SetComplexSampleRate( int complexSamplesPerSecond );
    int GetComplexSampleRate();
    int StartPrus();
    int GetRms( int nSamples, short *aSamples, double *rrms );
    int SetGain( int gn );
    int SetSim( int sim );

    // Mixer interfaces
    int64_t SetFrequency( int64_t freqHz );

    // Non ADC interface methods

    int GetFwVersion( );
    int SetSource( int arg );
    int SetTpg( int arg );

    // For testing only
    int  XspiWrite( int wval );
    void ShowPrus( const char *title );

    /**
     * Enumeration of valid data sources within fpga
     * to be used with SetSource()
     */
#   define XBOARD_FS_ADC    0
#   define XBOARD_FS_NCO1   1
#   define XBOARD_FS_NCO2   2
#   define XBOARD_FS_I      3
#   define XBOARD_FS_Q      4
#   define XBOARD_FS_CIC_I  5
#   define XBOARD_FS_CIC_Q  6
#   define XBOARD_FS_CIC_IQ 7

};

#endif
