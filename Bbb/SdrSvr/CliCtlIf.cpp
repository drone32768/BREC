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

#include "CliCtlIf.h"

CliCtlIf::CliCtlIf()
{
    mThreadExit = 0;
    mIpPort     = 8000;
    mRun        = 0;
}

//------------------------------------------------------------------------------
void CliCtlIf::SetSvcPort( int port )
{
    mIpPort = port;
}

//------------------------------------------------------------------------------
void CliCtlIf::SvcCmd( TcpSvrCon *tsc, Cli *cli )
{
    char          *cmdStr;
    const char    *rspStr;
    double         number;
    int            err;

    cmdStr = CliArgByIndex( cli, 0 );
    printf("Cmd= %s\n",cmdStr);        

    if( 0==strcmp(cmdStr,"inject")  ){
        char *arg1,*arg2,*arg3;
        char  lineBf[128];

        arg1 = CliArgByIndex( cli, 1 );
        arg2 = CliArgByIndex( cli, 2 );
        arg3 = CliArgByIndex( cli, 3 );
        sprintf(lineBf,"%s %s %s",
                        arg1?arg1:" ",
                        arg2?arg2:" ",
                        arg3?arg3:" "  );
        SndEvent(lineBf);
    }

    if( 0==strcmp(cmdStr,"help")  ){

        rspStr = "help - this command\n";
        TcpSvrWrite(tsc,rspStr,strlen(rspStr));

        rspStr = "read samples <N>  - read N samples and send\n";
        TcpSvrWrite(tsc,rspStr,strlen(rspStr));

        rspStr = "mcast-on  - starts sending samples via multicast\n";
        TcpSvrWrite(tsc,rspStr,strlen(rspStr));

        rspStr = "mcast-off  - stops sending samples via multicast\n";
        TcpSvrWrite(tsc,rspStr,strlen(rspStr));

        rspStr = "test pattern <N>  - sets test pattern data (0==off)\n";
        TcpSvrWrite(tsc,rspStr,strlen(rspStr));

        rspStr = "bits-per-sample  - returns quantity\n";
        TcpSvrWrite(tsc,rspStr,strlen(rspStr));

        rspStr = "samples-per-second - returns quantity\n";
        TcpSvrWrite(tsc,rspStr,strlen(rspStr));

        SndEvent("general.help");
    }

    if( 0==strcmp(cmdStr,"mcast-on")  ){
    }

    if( 0==strcmp(cmdStr,"mcast-off")  ){
    }

    if( 0==strcmp(cmdStr,"bits-per-sample")  ){
        rspStr = "12\n";
        TcpSvrWrite(tsc,rspStr,strlen(rspStr));
    }

    if( 0==strcmp(cmdStr,"samples-per-second")  ){
        // sprintf(ganOutBf,"%d\n", gnSamplesPerSecond );
        // TcpSvrWrite(tsc,ganOutBf,strlen(ganOutBf));
    }

    if( 0==strcmp(cmdStr,"test")  ){
        err = CliArgByNameAsNumber(cli, "pattern", &number );
        if( err ) {
            // gnTestPattern = 0;
        }
        else{
            // gnTestPattern = number;
        }
    }

}

//------------------------------------------------------------------------------
void CliCtlIf::RcvEvent( char *evtStr ) 
{
    printf("CliCtlIf : rcv <%s>\n",evtStr);
}

//------------------------------------------------------------------------------
void CliCtlIf::Main()
{
    int              err, rb, idx;
    char             peerStr[128];
    char             bf[128];
    Cli              *cli;
    TcpSvrCon        *tsc;

    printf("Entering main control loop\n");
    while( !mThreadExit ){

        printf("Allocate new control connect\n");
        tsc = TcpSvrConNew();
        if( !tsc ) return;

        printf("Waiting for cli control connection\n");
        err = TcpSvrConWaitNewCon(tsc,mIpPort);
        if( err<0 ) continue;

        TcpSvrGetClnIpStr( tsc, peerStr, sizeof(peerStr) );
        printf("Connection from %s\n",peerStr);

        printf("Servicing connection\n");
        cli = CliNew();
        if( !cli ) break;

        rb = 1;
        while( rb>0 ){
            
            printf("Waiting/read command data\n");
            rb = TcpSvrRead( tsc, bf, sizeof(bf) );
            if( rb<=0 ){
               break;
            }

            if( rb>0 ) bf[rb]=0;
    	    for( idx=0; idx<rb; idx++ ){
                err = CliInputCh( cli, bf[idx] );
                if( err > 0 ){
                    printf("Servicing command\n");
                    SvcCmd( tsc, cli );
                }
                if( err < 0 ){
                    break;
                }
            }		 

            // CliShow( cli );
            CliReset( cli );
        }

        TcpSvrClose( tsc );
        free( tsc );
        free( cli );
    }
}
