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

#ifndef __ASCP_UTIL_H__
#define __ASCP_UTIL_H__

#define ITEM_NAME        0x0001
#define ITEM_SN          0x0002
#define ITEM_INTERFACE   0x0003
#define ITEM_VER         0x0004
#define ITEM_STATUS      0x0005
#define ITEM_PRODUCT_ID  0x0009
#define ITEM_STATE       0x0018
#define ITEM_CHNLCFG     0x0019
#define ITEM_FREQ        0x0020
#define ITEM_RFGAIN      0x0038
#define ITEM_FILTER      0x0044
#define ITEM_ADMODE      0x008A
#define ITEM_SAMP_CAL    0x00B0
#define ITEM_PULSEMODE   0x00B6
#define ITEM_SAMPRATE    0x00B8

unsigned short GetLeUint16( unsigned char *bytes );
unsigned int   GetLeUint32( unsigned char *bytes );
void SetLeUint16( unsigned char *bytes, unsigned short v );
void SetAscpHdr( unsigned char *bf, int cmd, int item, int len );

#endif
