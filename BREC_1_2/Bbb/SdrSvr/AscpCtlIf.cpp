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

#include "../Util/net.h"
#include "AscpUtil.h"
#include "AscpDatIf.h"
#include "AscpCtlIf.h"

AscpCtlIf::AscpCtlIf()
{
    mThreadExit = 0;
    mIpPort     = 8000;
    mCmdBytes   = 0;
    mSampleRateHz = 2000000;
}

//------------------------------------------------------------------------------
void AscpCtlIf::SetSvcPort( int port )
{
    mIpPort = port;
}

//------------------------------------------------------------------------------
void AscpCtlIf::SetRadioName( char *radioName )
{
    char lineBf[128];

    snprintf(lineBf, sizeof(lineBf)-1,"radio-name %s", radioName);
    SndEvent( lineBf );
}

//------------------------------------------------------------------------------
void AscpCtlIf::SetState()
{
    int sndBytes;

    printf("SetState\n");
    sndBytes = 6;
    SetAscpHdr(  mRspMsg,    0, ITEM_STATE  , sndBytes );
 
    // TODO this just copies...
    mRspMsg[4] = mCmdMsg[4];
    mRspMsg[5] = mCmdMsg[5];
    mRspMsg[6] = mCmdMsg[6];
    mRspMsg[7] = mCmdMsg[7];

    // Save local run state and forward to dat
    if( mCmdMsg[5] == 1 ) { 
        SndEvent( "ascp.dat.halt" );
    }
    if( mCmdMsg[5] == 2 ) { 
        SndEvent( "ascp.dat.run" );
    }

    TcpSvrWrite(mTsc, (const char*)mRspMsg, sndBytes );
}

//------------------------------------------------------------------------------
uint64_t
DecodeLeUint40( unsigned char *ptr )
{
    uint64_t val;
    val  = ptr[0];
    val |= ptr[1]<<8;
    val |= ptr[2]<<16;
    val |= ptr[3]<<24;
    val |= (long long)(ptr[4])<<32;
    return( val );
}

//------------------------------------------------------------------------------
void AscpCtlIf::SetFreq()
{
    int sndBytes;
    long long freq;
    char lineBf[128];

    printf("SetFreq\n");
    sndBytes = 10;
    SetAscpHdr(  mRspMsg,    0, ITEM_FREQ  , sndBytes );
 
    freq = DecodeLeUint40( (mRspMsg+5) );
    printf("   freq=%f\n", (double) freq );

    sprintf(lineBf,"tune-hz %lld", freq);
    //sprintf(lineBf,"tune-hz %d", freq);
    SndEvent( lineBf );

    // TODO this just copies...
    mRspMsg[4] = mCmdMsg[4];
    mRspMsg[5] = mCmdMsg[5];
    mRspMsg[6] = mCmdMsg[6];
    mRspMsg[7] = mCmdMsg[7];
    mRspMsg[8] = mCmdMsg[8];
    mRspMsg[9] = mCmdMsg[9];

    TcpSvrWrite(mTsc, (char*)mRspMsg, sndBytes );
}

//------------------------------------------------------------------------------
void AscpCtlIf::SetChnlCfg()
{
    int sndBytes;

    printf("SetChnlCfg\n");
    sndBytes = 5;
    SetAscpHdr(  mRspMsg,    0, ITEM_CHNLCFG  , sndBytes );
 
    // TODO this just copies...
    mRspMsg[4] = mCmdMsg[4];

    TcpSvrWrite(mTsc, (char*)mRspMsg, sndBytes );
}

//------------------------------------------------------------------------------
void AscpCtlIf::SetFilter()
{
    int sndBytes;

    printf("SetFilter\n");
    sndBytes = 6;
    SetAscpHdr(  mRspMsg,    0, ITEM_FILTER  , sndBytes );
 
    // TODO this just copies...
    mRspMsg[4] = mCmdMsg[4];
    mRspMsg[5] = mCmdMsg[5];

    TcpSvrWrite(mTsc, (char*)mRspMsg, sndBytes );
}

//------------------------------------------------------------------------------
void AscpCtlIf::SetRfGain()
{
    int sndBytes;

    printf("SetRfGain\n");
    sndBytes = 6;
    SetAscpHdr(  mRspMsg,    0, ITEM_RFGAIN  , sndBytes );
 
    // TODO this just copies...
    mRspMsg[4] = mCmdMsg[4];
    mRspMsg[5] = mCmdMsg[5];

    TcpSvrWrite(mTsc, (char*)mRspMsg, sndBytes );
}

//------------------------------------------------------------------------------
void AscpCtlIf::SetAdMode()
{
    int sndBytes;

    printf("SetAdMode\n");
    sndBytes = 6;
    SetAscpHdr(  mRspMsg,    0, ITEM_ADMODE  , sndBytes );
 
    // TODO this just copies...
    mRspMsg[4] = mCmdMsg[4];
    mRspMsg[5] = mCmdMsg[5];

    TcpSvrWrite(mTsc, (char*)mRspMsg, sndBytes );
}

//------------------------------------------------------------------------------
void AscpCtlIf::SetPulseMode()
{
    int sndBytes;

    printf("SetPulseMode\n");
    sndBytes = 6;
    SetAscpHdr(  mRspMsg,    0, ITEM_PULSEMODE  , sndBytes );
 
    // TODO this just copies...
    mRspMsg[4] = mCmdMsg[4];
    mRspMsg[5] = mCmdMsg[5];

    TcpSvrWrite(mTsc, (char*)mRspMsg, sndBytes );
}

//------------------------------------------------------------------------------
void AscpCtlIf::SetSampRate()
{
    int sndBytes;
    char lineBf[128];

    printf("SetSampRate\n");
    sndBytes = 9;
    SetAscpHdr(  mRspMsg,    0, ITEM_SAMPRATE  , sndBytes );
 
    mSampleRateHz = GetLeUint32( mCmdMsg+5 );
    sprintf(lineBf,"sample-rate %d", mSampleRateHz);
    SndEvent( lineBf );

printf("0x%02x 0x%02x 0x%02x 0x%02x = %d\n",
    mCmdMsg[8], mCmdMsg[7], mCmdMsg[6], mCmdMsg[5],mSampleRateHz );

    // TODO this just copies...
    mRspMsg[4] = mCmdMsg[4]; // chnl

    mRspMsg[5] = mCmdMsg[5];
    mRspMsg[6] = mCmdMsg[6];
    mRspMsg[7] = mCmdMsg[7];
    mRspMsg[8] = mCmdMsg[8];

    TcpSvrWrite(mTsc, (char*)mRspMsg, sndBytes );
}

//------------------------------------------------------------------------------
void AscpCtlIf::GetName()
{
    int sndBytes;

    printf("GetName\n");
    sndBytes  = sprintf( (char*)(mRspMsg+4),"NetSDR"   );
    //sndBytes  = sprintf( (char*)(mRspMsg+4),"SDR-IP"   );
    sndBytes += (1+4);
    SetAscpHdr( mRspMsg, 0, ITEM_NAME, sndBytes );
    TcpSvrWrite(mTsc, (char*)mRspMsg, sndBytes );
}

//------------------------------------------------------------------------------
void AscpCtlIf::GetSn()
{
    int sndBytes;

    printf("GetSn\n");
    sndBytes = sprintf( (char*)(mRspMsg+4),"001"   );
    sndBytes += (1+4);
    SetAscpHdr( mRspMsg, 0, ITEM_SN  , sndBytes );
    TcpSvrWrite(mTsc, (char*)mRspMsg, sndBytes );
}

//------------------------------------------------------------------------------
void AscpCtlIf::GetVer()
{
    int sndBytes;

    printf("GetVer\n");
    sndBytes = 6;
    SetAscpHdr( mRspMsg,    0, ITEM_VER  , sndBytes );
    SetLeUint16( mRspMsg+4, 0         ); 
    TcpSvrWrite(mTsc, (char*)mRspMsg, sndBytes );
}

//------------------------------------------------------------------------------
void AscpCtlIf::GetStatus()
{
    int sndBytes;

//    printf("GetStatus\n");
    sndBytes = 5;
    SetAscpHdr( mRspMsg,    0, ITEM_STATUS  , sndBytes );
    mRspMsg[4] = 0xB; // Always return idle
    TcpSvrWrite(mTsc, (char*)mRspMsg, sndBytes );
}

//------------------------------------------------------------------------------
void AscpCtlIf::GetInterface()
{
    int sndBytes;

    printf("GetInterface\n");
    sndBytes = 6;
    SetAscpHdr( mRspMsg,    0, ITEM_INTERFACE  , sndBytes );
    mRspMsg[4] = 0x0; 
    mRspMsg[5] = 0x0; 
    TcpSvrWrite(mTsc, (char*)mRspMsg, sndBytes );
}

//------------------------------------------------------------------------------
void AscpCtlIf::GetProductId()
{
    int sndBytes;

    printf("ProductId\n");
    sndBytes = 8;
    SetAscpHdr( mRspMsg,    0, ITEM_PRODUCT_ID  , sndBytes );
    mRspMsg[4] = 0x0; 
    mRspMsg[5] = 0x0; 
    mRspMsg[6] = 0x0; 
    mRspMsg[7] = 0x0; 
    TcpSvrWrite(mTsc, (char*)mRspMsg, sndBytes );
}

//------------------------------------------------------------------------------
void AscpCtlIf::GetSampleRateCal()
{
    int sndBytes;

    printf("SampleRateCal\n");
    sndBytes = 8;
    SetAscpHdr( mRspMsg,    0, ITEM_SAMP_CAL  , sndBytes );
    mRspMsg[4] = 0x0; 
    mRspMsg[5] = 0x0; 
    mRspMsg[6] = 0x0; 
    mRspMsg[7] = 0x0; 
    TcpSvrWrite(mTsc, (char*)mRspMsg, sndBytes );
}

//------------------------------------------------------------------------------
void AscpCtlIf::RngFreq()
{
    int sndBytes;

    printf("RngFreq\n");
    sndBytes = 21;
    SetAscpHdr(  mRspMsg,    3, ITEM_FREQ  , sndBytes );
 
    // TODO hard coded frequency range
    mRspMsg[5] = 0;
    mRspMsg[6] = 1;

    mRspMsg[7]  = 0;
    mRspMsg[8]  = 0;
    mRspMsg[9]  = 0xAD;
    mRspMsg[10] = 0x86;
    mRspMsg[11] = 0x01;

    mRspMsg[12]  = 0;
    mRspMsg[13]  = 0x02;
    mRspMsg[14]  = 0x06;
    mRspMsg[15]  = 0xCC;
    mRspMsg[16]  = 0x80;

    mRspMsg[17]  = 0;
    mRspMsg[18]  = 0;
    mRspMsg[19]  = 0;
    mRspMsg[20]  = 0;
    mRspMsg[21]  = 0;

    TcpSvrWrite(mTsc, (char*)mRspMsg, sndBytes );
}

//------------------------------------------------------------------------------
void AscpCtlIf::CtlParseInputMsg( int cmd, int item, int len )
{
    int parsed = 1;

    // printf("ParseInputMsg:cmd=%d item=%d len=%d\n",cmd,item,len);
    switch( cmd ){

        // ------ Set
        case 0x00 :{
            switch( item ){
                case ITEM_STATE      : { SetState();      break; }
                case ITEM_FREQ       : { SetFreq();       break; }
                case ITEM_CHNLCFG    : { SetChnlCfg();    break; }
                case ITEM_FILTER     : { SetFilter();     break; }
                case ITEM_RFGAIN     : { SetRfGain();     break; }
                case ITEM_ADMODE     : { SetAdMode();     break; }
                case ITEM_PULSEMODE  : { SetPulseMode();  break; }
                case ITEM_SAMPRATE   : { SetSampRate();   break; }
                default              : parsed=0;
            }
            break;
        } // End of cmd == set

        // ------ Get
        case 0x01 :{
            switch( item ){
                case ITEM_NAME       : { GetName();    break; }
                case ITEM_SN         : { GetSn();      break; }
                case ITEM_VER        : { GetVer();     break; }
                case ITEM_STATUS     : { GetStatus();  break; }
                case ITEM_INTERFACE  : { GetInterface();  break; }
                case ITEM_PRODUCT_ID : { GetProductId();  break; }
                case ITEM_SAMP_CAL   : { GetSampleRateCal();  break; }
                default              : parsed=0;
            }
            break;
        } // End of cmd == get

        // ------ Range
        case 0x02 :{
            switch( item ){
                case ITEM_FREQ       : { RngFreq(); break; }
                default              : parsed=0;
            }
            break;
        } // End of cmd = range

        // ------ ???
        default : parsed=0;

    } // End of switch over cmd

    if( !parsed ){
        printf("unparsed msg: %d %d %d\n",cmd,item,len);
    }
}

//------------------------------------------------------------------------------
void AscpCtlIf::CtlParseInputByte( unsigned char byte )
{
    unsigned int len;
    int cmd;
    int item;
    unsigned short w0;

    mCmdMsg[ mCmdBytes++ ] = byte;
    if( mCmdBytes > 3 ){
        w0   = GetLeUint16(mCmdMsg);
        item = GetLeUint16(mCmdMsg + 2 );
        len  = w0 & 0x1fff;
        cmd  = (w0>>13)&0x7;
        if( len == mCmdBytes ){
            CtlParseInputMsg( cmd, item, len );
            mCmdBytes = 0;
        }
    }
    if( mCmdBytes > sizeof(mCmdMsg) ){
        printf("input msg exceeds buffer\n");
        mCmdBytes = 0;
    }
}

//------------------------------------------------------------------------------
void AscpCtlIf::RcvEvent( char *evtStr )
{
    printf("AscpCtlIf: rcv <%s>\n",evtStr);
}

//------------------------------------------------------------------------------
void AscpCtlIf::Main()
{
    int              err, rb;
    char             ch;
    char             peerStr[128];
    char             lineBf[128];

    printf("Entering main control loop\n");
    while( !mThreadExit ){

        printf("Allocate new control connect\n");
        mTsc = TcpSvrConNew();
        if( !mTsc ) return;

        err = TcpSvrConBind(mTsc,mIpPort);
        if( err<0 ) continue;

        sprintf(lineBf,"ascp.dis.set-port %d", mIpPort);
        SndEvent( lineBf );

        printf("Waiting for ascp control connection\n");
        err = TcpSvrConAccept(mTsc);
        if( err<0 ) continue;

        TcpSvrGetClnIpStr( mTsc, peerStr, sizeof(peerStr) );
        printf("Connection from %s\n",peerStr);

        sprintf(lineBf,"ascp.dat.set-host %s", peerStr);
        SndEvent( lineBf );

        sprintf(lineBf,"ascp.dat.set-port %d", mIpPort);
        SndEvent( lineBf );

        sprintf(lineBf,"dev-complex");
        SndEvent( lineBf );

        while( 1 ){
            // printf("Reading input...\n");
            rb = TcpSvrRead( mTsc, &ch, 1 );
            if( rb <= 0 ) break;
            CtlParseInputByte( (unsigned char)ch );
        }
       
        printf("Client connection closed.\n");
        TcpSvrClose( mTsc );
    }
}

