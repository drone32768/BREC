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
#ifndef __INSTMODEL_H__
#define __INSTMODEL_H__

#include "Util/mcf.h"
#include "Pse.h"

/**
 * This class represents the instrument model.
 * It has its own thread for scanning (as derived from McF).
 */
class InstModel : public McF {

private:
#   define         HWM_LOG_SETOUTPUT 0x00000010
#   define         HWM_LOG_SWEEP     0x00000020
    unsigned int   mLog;   // Logging level for hw model

    int            mRun;         // on/off running with hw
    int            mParamChange; // Flag indicating state has changed
    char          *mCfgFname;

    int            mXyCurLen;    // number of points in xy vectors
    int            mXyMaxLen;    // maximum points in xy vectors

    double         *mXvec;       // x values
    double         *mYvec;       // y values

    double         mXmin,mXmax;  // x limits determined by processing

    int            mScanReset;
    Pse            mPse;         // power spectrum estimator object
    double         mCenterHz;    // center frequency
    double         mSpanHz;      // span frequency

    // Internal support routines
    int            HwInit();
    void           HwStop();
    void           ShowState();
    int            ReadCfg();
    int            WriteCfg();

    void           ScanReset();
    void           ScanSvc();
    void           ScanStep();

public:

    // Zero arg constructor
    InstModel();

    // Reqired interface elements for mcf
    void   RcvEvent( char *evtStr );
    void   Main();

    // Primary public interface
    int    SetState( char *name, char *value );
    int    GetState( char *resultsStr, int resultsLen );
    int    GetData( char *resultsStr, int resultsLen );
    int    SetCfg( const char *fname );
};

#endif // __INSTMODEL_H__
