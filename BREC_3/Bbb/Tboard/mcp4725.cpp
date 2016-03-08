//
//
// This source code is available under the "Simplified BSD license".
//
// Copyright (c) 2015, J. Kleiner
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "Util/mcf.h"
#include "Util/gpioutil.h"
#include "Ui2c/ui2c.h"

#include "mcp4725.h"

////////////////////////////////////////////////////////////////////////////////
MCP4725::MCP4725()
{
   mDbg = 0x0;
}

////////////////////////////////////////////////////////////////////////////////
uint32_t 
MCP4725::Configure( UI2C *ui2c )
{
   mI2c = ui2c;
   return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
uint32_t
MCP4725::Dbg( uint32_t dbg )
{
   uint32_t pdbg;
   pdbg   = mDbg;
   mDbg   = dbg;
   return( pdbg );
}

////////////////////////////////////////////////////////////////////////////////
uint32_t 
MCP4725::Set( uint8_t devAddr, int value )
{
    int           err = 0;

    // register address is mix of pd and high bits
    // second byte is residual of value
    uint8_t b1 = (value>>8) & 0x0f;
    uint8_t b2 = (value   ) & 0xff;

    err = mI2c->start_cond();
    if( err && mDbg ){
        printf("%s:%d: err on start 0x%08x\n",__FILE__,__LINE__,err);
        return( err );
    }

    err = mI2c->write_cycle( (devAddr&0xfe) );
    if( err && mDbg ){
        printf("%s:%d: err dev addr write cycle 0x%08x\n",
                             __FILE__,__LINE__,err);
        return( err );
    }

    err = mI2c->write_cycle( b1 );
    if( err && mDbg ){
        printf("%s:%d: err b1 write cycle 0x%08x\n",
                            __FILE__,__LINE__,err);
        return( err );
    }

    err = mI2c->write_cycle( b2 );
    if( err && mDbg ){
        printf("%s:%d: err b2 write cycle 0x%08x\n",
                            __FILE__,__LINE__,err);
        return( err );
    }

    err = mI2c->stop_cond();
    if( err && mDbg ){
        printf("%s:%d: err stop_cond 0x%08x\n",__FILE__,__LINE__,err);
        return( err );
    }
 
    return( err );
}

////////////////////////////////////////////////////////////////////////////////
uint32_t 
MCP4725::Get( uint8_t devAddr, int *vvalue )
{
    int           err = 0;
    int           idx;
    uint8_t       bytes[6];

    err = mI2c->start_cond();
    if( err && mDbg ){
        printf("%s:%d: err on start 0x%08x\n",__FILE__,__LINE__,err);
        return( err );
    }

    err = mI2c->write_cycle( devAddr | 0x01 );
    if( err && mDbg ){
        printf("%s:%d: err dev addr write cycle 0x%08x\n",
                             __FILE__,__LINE__,err);
        return( err );
    }

    for( idx=0; idx<5; idx++ ){
        err = mI2c->read_cycle( &(bytes[idx]), 0 );
        if( err && mDbg ){
            printf("%s:%d: err b[%d] read cycle 0x%08x\n",
                                __FILE__,__LINE__,idx,err);
            return( err );
        }
        if( mDbg & MCP4725_DBG_GET ){
            printf("%s:%d: read byte[%d] 0x%02x\n",
                 __FILE__,__LINE__,idx,bytes[idx]);
        }
    }

    err = mI2c->stop_cond();
    if( err && mDbg ){
        printf("%s:%d: err stop_cond 0x%08x\n",__FILE__,__LINE__,err);
        return( err );
    }
 
    *vvalue = bytes[1];
    *vvalue = ( (*vvalue)<<4) + (bytes[2]>>4);
    return( err );
}
