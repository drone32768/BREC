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
#ifndef __SI_CTLIF_H__
#define __SI_CTLIF_H__

#include "../Util/mcf.h"
#include "../Util/net.h"
#include "../Util/cli.h"
#include "Devs.h"
#include "RemoteDev.h"
#include "SiEstimate.h"
#include "SiCal.h"

class SiCtlIf : public McF {

  private:
    /** Port number to use in servicing client connections */
    int           mIpPort;

    /** Flag indicating processing should be enabled */
    int           mRun;

    /** Flag indicating that frequency scanning should proceed */
    int           mDoScan;

    /** Flag indicating that a new scan should begin */
    int           mScanReset;

    /** Cumulative count of scans conducted */
    int           mScanCount;

    /** TBD */
    struct timeval mScanCountTvBase;

    /** Internal debug limit on scans to conduct before auto stop */
    int           mScanLimit;

    /** Sample buffer */
    short        *mSamples;

    /** Number of samples to collect in sample buffer for processing */
    int           mSampleCnt;

    /** Lower frequency limit in Hertz of scan begin */
    double        mFminHz;

    /** Upper frequency limit in Hertz of scan end */
    double        mFmaxHz;

    /** Current frequency setting of hardware */
    double        mFreqHz;

    /** Last programmed frequency setting of hardware */
    double        mLastFreqHz;

    /** Indicates how many IF stages are configured */
    int           mNif;

    /** IF frequencies in Hz */
#   define MAX_IFS 3
    double        mIFHz[ MAX_IFS ];

    /** LO frequencies in Hz */
    double        mLOHzAct[ MAX_IFS ];

    /** Indicates tracking generator points/steps per span */
    int           mTgPoints;

    /** Indicates IF gain control setting */
    int           mIfGain;

    /** Indicates RF channel selection */
    int           mRfCh;
 
    /** Indicates measurement bandwidth selection */
    int           mMbwKhz;

    /** Indicates number of fft points for baseline processing */
    int           mFftn;

    /** Indicates number of averages for baseline processing */
    int           mAvne;

    /** Indicates if bin integration should be performed */
    int           mInt;

    /** Indicates attenuation setting (with correct RF channel selection) */
    double        mAttenDb;

    /** Control interface to external tracking generator */
    RemoteDev     mRemoteTg;

    /** Control interface to external local oscillators */
    RemoteDev     mRemoteLo[MAX_IFS];

    /** Frequency step in Hz */
    int           mFstepHz;

    /** FFT size in use */
    int           mFftSize;

    /** Spectral Estimation object */
    SiEstimator   mSiEst;

    /** Counters relating to synthesizer lock */
    int           mLockWait;
    int           mLockWaitAbort;

    /** Various stop watches for performance/timing evaluation */
    StopWatch     mSvcScanSw;
    StopWatch     mAdcSw;
    StopWatch     mSiSw;
    StopWatch     mNetSw;
    StopWatch     mFreqSw;

    /** Calibration object */
    SiCal         mCal;

    /** Internal support methods */
    void      SvcErr( TcpSvrCon *tsc );
    void      SvcCmd(  TcpSvrCon *tsc, Cli *cli );
    int       SvcCtl(  TcpSvrCon *tsc, Cli *cli );
    int       SvcScan( TcpSvrCon *tsc );
    void      DoScanReset( TcpSvrCon *tsc );
    double    SetLoWithLockWait( int lon, double fHz );
    double    SetRemoteTG( double fHz );
    double    SetRemoteLO( int ln, double fHz );
    void      SetDefaultIFs();
    void      ConfigureStaticLOs();
    void      ConfigureHw();

  public:

    // MCF required interfaces
    SiCtlIf();
    void      Main();
    void      RcvEvent( char *evtStr );
    void      SetSvcPort( int port );
};

#endif
