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
#include <ifaddrs.h>

#include "../Util/net.h"
#include "AscpDisIf.h"

AscpDisIf::AscpDisIf()
{
#   define DIS_LOG_EVT 0x00000001
    mLog        = 0xfffff;
    mThreadExit = 0;
    mIpPort     = 0;
    mRadioName  = strdup("default");
}

//------------------------------------------------------------------------------
void AscpDisIf::RcvEvent( char *evtStr )
{
    char *cmdStr;
    char *argStr;
    char *tokr;


    if( mLog&DIS_LOG_EVT ){
       printf("AscpDisIf:RcvEvent <%s>\n",evtStr);
    }

    cmdStr = strtok_r( evtStr, " ", &tokr );
   
    if( 0==strcmp("ascp.dis.set-port",cmdStr) ){
       argStr = strtok_r(NULL, " ", &tokr);
       mIpPort =  atoi( argStr );
       if( mLog&DIS_LOG_EVT ){
          printf("AscpDisIf:Port is now <%d>\n",mIpPort);
       }
    }

    if( 0==strcmp("radio-name",cmdStr) ){
       argStr = strtok_r(NULL, " ", &tokr);
       mRadioName =  strdup(argStr);
       if( mLog&DIS_LOG_EVT ){
          printf("AscpDisIf:Radio name is <%s>\n",mRadioName);
       }
    }
}

//------------------------------------------------------------------------------
void AscpDisIf::SendResponse( UdpSvr *usv )
{
    int           nBytes;
    int           ret;
    unsigned char sndBytes[128];

    if( mLog&DIS_LOG_EVT ){
       printf("AscpDisIf:Discover response %s\n",mRadioName);
    }
    
    nBytes = 56;
        
    sndBytes[0] = (unsigned char)( nBytes&0xff);
    sndBytes[1] = (unsigned char)( (nBytes>>8)&0xff );
    
    sndBytes[2] = 0x5A;
    sndBytes[3] = 0xA5;
        
    sndBytes[4] = 1; // 1=response
        
    snprintf((char*)(sndBytes+5 ), 15, "%s",mRadioName ); 
    snprintf((char*)(sndBytes+21), 15, "SerialNo" ); 
        
    sndBytes[37]= 0; // lsb ipv4 addr
    sndBytes[38]= 0;
    sndBytes[39]= 0;
    sndBytes[40]= 0; // msb ipv4 addr
        
    sndBytes[53]= (mIpPort&0xff);
    sndBytes[54]= ( (mIpPort>>8)&0xff );
        
    sndBytes[55]=0;
   
    // Send the response on all interfaces
    struct ifaddrs *ifap,*ifa;
    char   ipStr[128];

    ret = getifaddrs( &ifap );
    if( ret<0 ){
        perror("getifaddrs");
    }
    for( ifa=ifap; ifa!=NULL; ifa=ifa->ifa_next ){
        if( ifap->ifa_addr->sa_family == AF_INET ){ 
           struct in_addr *inAddr;

            inAddr = &( ((struct sockaddr_in*)(ifap->ifa_addr))->sin_addr ); 
            if( !inet_ntop( AF_INET,
                            inAddr,
                            ipStr,sizeof(ipStr)) ){
                perror("inet_ntop");
            }
            if( mLog&DIS_LOG_EVT ){
               printf("AscpDisIf:Respond on %s/%s\n",ifa->ifa_name,ipStr);
            }

            // Tailor ip svc address to this interface
            sndBytes[40] = (inAddr->s_addr    ) & 0xff;
            sndBytes[39] = (inAddr->s_addr>>8 ) & 0xff;
            sndBytes[38] = (inAddr->s_addr>>16) & 0xff;
            sndBytes[37] = (inAddr->s_addr>>24) & 0xff;

            inAddr= &( ((struct sockaddr_in*)(ifap->ifa_broadaddr))->sin_addr); 
            if( !inet_ntop( AF_INET,
                            inAddr,
                            ipStr,sizeof(ipStr)) ){
                perror("inet_ntop");
            }
            if( mLog&DIS_LOG_EVT ){
               printf("AscpDisIf:   Broadcast response to %s\n",ipStr);
            }

            // Send response to this interfaces broadcast address
            UdpSvrSendto( usv, ipStr, 48322, sndBytes, nBytes );

        }
        ifap = ifap->ifa_next;
    }
    freeifaddrs(ifap);
}

//------------------------------------------------------------------------------
void AscpDisIf::Main()
{
    UdpSvr          *usv;
    unsigned char    bf[1024];
    int              nr;

    usv = UdpSvrNew( 48321, 1 );
    if( !usv ){
       fprintf(stderr,"failed to establish discovery socket\n");
       return;
    }

    while( !mThreadExit ){
        nr = UdpSvrRecv( usv, bf, sizeof(bf) );
        if( nr == 56 ) SendResponse( usv );
    }
}

