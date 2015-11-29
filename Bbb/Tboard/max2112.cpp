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

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"
#include "ui2c.h"

#include "max2112.h"

////////////////////////////////////////////////////////////////////////////////
MAX2112::MAX2112()
{
   mDbg = 0x0;
}

////////////////////////////////////////////////////////////////////////////////
uint32_t 
MAX2112::Configure( UI2C *ui2c )
{
   mI2c = ui2c;
   return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
uint32_t 
MAX2112::Write( 
   uint8_t  devAddr, 
   uint8_t  regAddr,  
   uint8_t *regBytes, 
   int      nBytes )
{
    int           err = 0;
    int           cerr;
    int           idx;

    cerr = mI2c->start_cond();
    err |= cerr;
    if( cerr && mDbg ){
        printf("%s:%d: err start[1] 0x%08x\n",__FILE__,__LINE__,cerr);
        return( cerr );
    }

    cerr = mI2c->write_cycle( (devAddr&0xfe) );
    err |= cerr;
    if( cerr && mDbg ){
        printf("%s:%d: err dev addr write cycle 0x%08x\n",
                             __FILE__,__LINE__,cerr);
        return( cerr );
    }

    cerr = mI2c->write_cycle( (regAddr&0xff) );
    err |= cerr;
    if( cerr && mDbg ){
        printf("%s:%d: err reg addr write cycle 0x%08x\n",
                            __FILE__,__LINE__,cerr);
        return( cerr );
    }

    for(idx=0;idx<nBytes;idx++){
        cerr = mI2c->write_cycle( regBytes[idx] );
        err |= cerr;
        if( cerr && mDbg ){
            printf("%s:%d: err value write cycle idx=%d/%d 0x%08x\n",
                            __FILE__,__LINE__,cerr,idx,nBytes);
        return( cerr );
        }
    }

    cerr = mI2c->stop_cond();
    err |= cerr;
    if( cerr && mDbg ){
        printf("%s:%d: err stop_cond 0x%08x\n",__FILE__,__LINE__,cerr);
        return( cerr );
    }
 
    return( err );
}

////////////////////////////////////////////////////////////////////////////////
uint32_t 
MAX2112::Read( 
   uint8_t  devAddr, 
   uint8_t  regAddr, 
   uint8_t *regBytes, 
   int      nBytes )
{
    int           err = 0;
    int           cerr;
    int           idx;

    cerr = mI2c->start_cond();
    err |= cerr;
    if( cerr && mDbg ){
        printf("%s:%d: start[1] 0x%08x\n",__FILE__,__LINE__,cerr);
    }

    cerr = mI2c->write_cycle( (devAddr&0xfe) );
    err |= cerr;
    if( cerr && mDbg ){
        printf("%s:%d: dev addr write cycle 0x%08x\n",__FILE__,__LINE__,cerr);
    }

    cerr = mI2c->write_cycle( (regAddr&0xff) );
    err |= cerr;
    if( cerr && mDbg ){
        printf("%s:%d: reg addr write cycle 0x%08x\n",__FILE__,__LINE__,cerr);
    }

    cerr = mI2c->start_cond();
    err |= cerr;
    if( cerr && mDbg ){
        printf("%s:%d: start[2] 0x%08x\n",__FILE__,__LINE__,cerr);
    }

    cerr = mI2c->write_cycle( (devAddr&0xfe) | 1 );
    err |= cerr;
    if( cerr && mDbg ){
        printf("%s:%d: write cycle R=1 0x%08x\n",__FILE__,__LINE__,cerr);
    }

    for(idx=0;idx<nBytes;idx++){
        cerr = mI2c->read_cycle( &(regBytes[idx]), (idx==(nBytes-1)?1:0) );
        err |= cerr;
        if( cerr && mDbg ){
            printf("%s:%d: read cycle idx=%d/%d 0x%08x\n",
                            __FILE__,__LINE__,cerr,idx,nBytes);
        }
    }

    cerr = mI2c->stop_cond();
    err |= cerr;
    if( cerr && mDbg ){
        printf("%s:%d: stop_cond 0x%08x\n",__FILE__,__LINE__,cerr);
    }

    return( err );
}

