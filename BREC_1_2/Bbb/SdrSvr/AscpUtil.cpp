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
#include "AscpUtil.h"

//------------------------------------------------------------------------------
unsigned short GetLeUint16( unsigned char *bytes )
{
    unsigned short w;
    w  = bytes[1];
    w  = w<<8;
    w |= bytes[0];
    return( w );
}

//------------------------------------------------------------------------------
unsigned int GetLeUint32( unsigned char *bytes )
{
    unsigned int w;
    w  = bytes[3];
    w  = w<<8;
    w |= bytes[2];
    w  = w<<8;
    w |= bytes[1];
    w  = w<<8;
    w |= bytes[0];
    return( w );
}

//------------------------------------------------------------------------------
void SetLeUint16( unsigned char *bytes, unsigned short v )
{
    bytes[0] = v&0xff;
    bytes[1] = (v>>8)&0xff;
}

//------------------------------------------------------------------------------
void SetAscpHdr( unsigned char *bf, int cmd, int item, int len )
{
    unsigned short w0;

    w0 = (cmd & 0x7)<<13;
    w0 = w0 | (len&0x1fff);
    SetLeUint16( bf,   w0 );
    SetLeUint16( bf+2, item );
}

