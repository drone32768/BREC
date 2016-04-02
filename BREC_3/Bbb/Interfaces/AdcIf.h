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
#ifndef __ADC_IF__
#define __ADC_IF__

class AdcIf {

public:

    /** Required first operation */
    virtual int Open() = 0;

    /** Starts internal processing for PRU based interfaces */
    virtual int StartPrus() = 0;

    /** Flush all hw and sw queued data */
    virtual int FlushSamples() = 0;

    /** 
     * Collects 2k short data words 
     *  [ slight misnomer - implies real valued samples but
     *  complex formats provide 1k samples/2k words ]
     */
    virtual int Get2kSamples( short *bf ) = 0; 

    /** TODO obsolete these */
    virtual int SetComplexSampleRate( int complexSamplesPerSecond ) = 0;
    virtual int GetComplexSampleRate( ) = 0;

    /** Internal gain control */ 
    virtual int SetGain( int gn ) = 0;

    /** Returns non zero if complext */
    virtual int IsComplexFmt() = 0;

    /** Returns source set */
    virtual int SetSource( int sn ) = 0;  

    /** Returns internal test pattern generator */
    virtual int SetTpg( int tp ) = 0;  

    /** Returns status of operation */
    virtual int SetChannelMatch(
              int Ioff,double Igain, int Qoff,double Qgain) = 0;

    /** TODO GetSource? */
};

#endif
