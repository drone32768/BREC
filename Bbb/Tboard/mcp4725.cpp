
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"
#include "ui2c.h"

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
MCP4725::Write( 
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
MCP4725::Read( 
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

