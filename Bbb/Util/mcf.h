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

#ifndef __MCF_H__
#define __MCF_H__

#include <sys/time.h>

// Minimal control framework

// Forward decl
class EvR;

// Minimal control framework base class
class McF {

    public:
    int   mThreadExit;
    EvR  *mEvr;

    int  Start();
    int  Stop();
    void SndEvent( const char *evtStr ); 

    public:
    McF();                 
    ~McF();
    virtual void Main() = 0;                 
    virtual void RcvEvent( char *evtStr ) = 0; 
};

// Event Registry
class EvR {
    private:
#   define EVT_MAX_HANDLERS 32
    McF *mHandlers[EVT_MAX_HANDLERS];

    public:
    EvR();
    int  Register( McF *obj );
    void Inject( McF *src, const char *evtStr );
};

// Various support routines
double             atof_suffix( const char *str );
int                tv_delta_useconds(struct timeval *tv2,struct timeval *tv1 );
unsigned long long tv_delta_nseconds(struct timeval *tv2,struct timeval *tv1 );
int                us_sleep( unsigned int us );
EvR                *GetEvrSingleton();
int                FindCapeByName( const char *findStr );

////////////////////////////////////////////////////////////////////////////////
/**
 * StopWatch - this utility class provides simple and common timing functions
 * for performance evaluations.
 */
class StopWatch {
    struct timeval tv1;
    struct timeval tv2;
    int            mIdx;
#   define STOP_WATCH_HISTORY_LEN 128
    unsigned long  mHistory[ STOP_WATCH_HISTORY_LEN ]; 

    public:
                  StopWatch();
    void          Start();
    void          Stop();
    unsigned long GetuS();
    void          StopWithHistory();
    unsigned long MedianuS();
};

#endif /* __MCF_H__ */

