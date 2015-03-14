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
#include <unistd.h>
#include <pthread.h>

#include "AdcCtlIf.h"

AdcCtlIf::AdcCtlIf()
{
    mThreadExit = 0;
    mIpPort     = 8000;
}

//------------------------------------------------------------------------------
void AdcCtlIf::SetSvcPort( int port )
{
    mIpPort = port;
}

//------------------------------------------------------------------------------
void AdcCtlIf::RcvEvent( char *evtStr )
{
    printf("AdcCtlIf : rcv <%s>\n",evtStr);
}

//------------------------------------------------------------------------------
void AdcCtlIf::SvcCmd(  TcpSvrCon *tsc, Cli *cli )
{
    char         *cmdStr;
    const char   *rspStr;
    char         lineBf[128];

    cmdStr = CliArgByIndex( cli, 0 );
    printf("Cmd=\"%s\"\n",cmdStr);        

    if( 0==strcmp(cmdStr,"help")  ){

        rspStr = "help - this command\n";
        TcpSvrWrite(tsc,rspStr,strlen(rspStr));

        rspStr = "adc-samples-per-second - returns quantity\n";
        TcpSvrWrite(tsc,rspStr,strlen(rspStr));
    }

    if( 0==strcmp(cmdStr,"adc-samples-per-second")  ){
        int csps;
        csps = Dp()->Adc()->GetComplexSampleRate();
        printf("Complex Samples Per Second = %d\n",csps);
        sprintf(lineBf,"%d\n",2*csps ); 
        TcpSvrWrite(tsc,lineBf,strlen(lineBf));
    }
}

//------------------------------------------------------------------------------
void AdcCtlIf::Main()
{
    int              err, rb;
    char             ch;
    char             lineBf[128]; 
    char             peerStr[128]; 
    Cli              *cli;

    printf("AdcCtlIf:Entering main control loop\n");
    while( !mThreadExit ){

        printf("AdcCtlIf:Allocate new control connect\n");
        mTsc = TcpSvrConNew();
        if( !mTsc ) return;

        err = TcpSvrConBind(mTsc,mIpPort);
        if( err<0 ) continue;

        printf("AdcCtlIf:Waiting for adc control connection p=%d\n",mIpPort);
        err = TcpSvrConAccept(mTsc);
        if( err<0 ) continue;

        TcpSvrGetClnIpStr( mTsc, peerStr, sizeof(peerStr) );
        printf("AdcCtlIf:Connection from %s\n",peerStr);

        sprintf(lineBf,"dev-real"); 
        SndEvent( lineBf );

        sprintf(lineBf,"adc.dat.set-host %s", peerStr);
        SndEvent( lineBf );

        sprintf(lineBf,"adc.dat.set-port %d", mIpPort);
        SndEvent( lineBf );

        sprintf(lineBf,"adc.dat.run");
        SndEvent( lineBf );

        cli = CliNew();
        while( 1 ){
            rb = TcpSvrRead( mTsc, &ch, 1 );
            if( rb <= 0 ) break;
            err = CliInputCh( cli, ch );
            if( err > 0 ){
               SvcCmd( mTsc, cli );
               CliReset(cli);
            }
        }
        free( cli );

        sprintf(lineBf,"adc.dat.halt");
        SndEvent( lineBf );
       
        printf("Client connection closed.\n");
        TcpSvrClose( mTsc );
    }
}

