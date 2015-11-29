#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"
#include "ui2c.h"

////////////////////////////////////////////////////////////////////////////////
UI2C::UI2C()
{
    mUsHold   = 100;
    mDbg      = 0x0;
    mGpioSCL  = NULL;
    mGpioSDA  = NULL;
}

////////////////////////////////////////////////////////////////////////////////
uint32_t
UI2C::Dbg( uint32_t dbg )
{
   uint32_t pdbg;
   pdbg   = mDbg;
   mDbg   = dbg;
   return( pdbg );
}

////////////////////////////////////////////////////////////////////////////////
uint32_t 
UI2C::wait_high( GpioUtil *pin )
{
   int cnt;

   pin->SetDirInput( 1 );
   cnt = 10;
   while( cnt>0 ){
      if( 1==pin->Get() ) return(0);
      us_sleep( 100 );
      cnt--;
   }
   return( UI2C_ERR_HIGH_TOUT );
}

////////////////////////////////////////////////////////////////////////////////
uint32_t 
UI2C::read_value( GpioUtil *pin )
{
   pin->SetDirInput(1);
   return( pin->Get() );
}

////////////////////////////////////////////////////////////////////////////////
uint32_t 
UI2C::pull_low( GpioUtil *pin )
{
   pin->SetDirInput(0);
   pin->Set(0);
   return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
uint32_t
UI2C::configure( GpioUtil *scl, GpioUtil *sda )
{
    // Save the two pins
    mGpioSCL = scl;
    mGpioSDA = sda;
 
    // Sanity check on init
    if( !mGpioSCL || !mGpioSDA ){
        printf("%s:%d gpios null\n",__FILE__,__LINE__);
        return( UI2C_ERR_CFG );
    }

    if( mDbg & UI2C_DBG_CFG ){
        printf("%s:%d:configure SCL=%d, SDA=%d\n",
                  __FILE__,__LINE__,
                  read_value(mGpioSCL),
                  read_value(mGpioSDA)  );
    }

    return(0);
}

////////////////////////////////////////////////////////////////////////////////
uint32_t
UI2C::start_cond()
{
    int err;

    if( mDbg & UI2C_DBG_START ){
        printf("%s:%d:start_cond issued \n",__FILE__,__LINE__);
    }

    // Make sure SCL is high
    err = wait_high( mGpioSCL );
    if( err ){
        return( UI2C_ERR_START_COND );
    }

    // Make sure SDA is high
    err = wait_high( mGpioSDA );
    if( err ){
        return( UI2C_ERR_START_COND );
    }

    // Wait min time with SDA and SCL high
    us_sleep( mUsHold );

    // Pull SDA low
    pull_low( mGpioSDA );
    us_sleep( mUsHold );
   
    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
uint32_t
UI2C::stop_cond()
{
    int err;

    if( mDbg & UI2C_DBG_START ){
        printf("%s:%d:stop_cond issued \n",__FILE__,__LINE__);
    }
   
    // Pull clock low so we can change data
    pull_low( mGpioSCL );
    us_sleep( mUsHold );

    // Pull data low
    pull_low( mGpioSDA );
    us_sleep( mUsHold );

    // Allow clock high
    err = wait_high( mGpioSCL );
    if( err ){
        return( UI2C_ERR_STOP_COND );
    }
    us_sleep( mUsHold );

    // Allow data to transition high while clock high thus signalling stop
    err = wait_high( mGpioSDA );
    if( err ){
        return( UI2C_ERR_STOP_COND );
    }
    us_sleep( mUsHold );

    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
uint32_t
UI2C::write_cycle( uint8_t byte )
{
    int           err = 0;
    int           cnt;
    int           ab;
    int           bit;

    if( mDbg & UI2C_DBG_WRITE_CYCLE ){
        printf("%s:%d:write_cycle begin: val=0x%02x\n",
                  __FILE__,__LINE__,byte);
    }

    for(cnt=0;cnt<8;cnt++){

        // SCL low
        pull_low( mGpioSCL );
        us_sleep( mUsHold );

        // SDA transition
        bit = ((byte&0x80)>>7);
        if( mDbg & UI2C_DBG_WRITE_CYCLE ) printf("%d ",bit);
        if( bit ){
           wait_high( mGpioSDA );
        }
        else{
           pull_low( mGpioSDA );
        }
        us_sleep( mUsHold );

        // SCL high
        err = wait_high( mGpioSCL );
        if( err ){
            return( UI2C_DBG_WRITE_CYCLE );
        }
        us_sleep( mUsHold );

        byte = byte << 1;
    }
    if( mDbg & UI2C_DBG_WRITE_CYCLE ) printf("\n");

    // SCL is high at this point

    // Pull SCL low so slave can drive SDA
    // And turn SDA into an input (to stop driving it)
    pull_low( mGpioSCL );
    mGpioSDA->SetDirInput(1);   
    us_sleep( mUsHold );

    // SCL high
    err = wait_high( mGpioSCL );

    // Get SDA state to check slave ACK
    ab = mGpioSDA->Get();       
    if( ab ){
        return( UI2C_DBG_WRITE_CYCLE );
    }

    if( mDbg & UI2C_DBG_WRITE_CYCLE ){
        printf("%s:%d:write_cycle end  : err=%d\n",
              __FILE__,__LINE__,err);
    }

    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
uint32_t
UI2C::read_cycle( uint8_t *bytePtr, int nack )
{
    int           err = 0;
    int           cnt;
    int           bit;
    uint8_t       byte = 0;

    // FIXME - update this routine

    mGpioSDA->SetDirInput(1);
    us_sleep( mUsHold );  

    for(cnt=0;cnt<8;cnt++){

        mGpioSCL->Set(0);
        us_sleep( mUsHold );

        mGpioSCL->Set(1);
        us_sleep( mUsHold );

        bit = mGpioSDA->Get();
        us_sleep( mUsHold );

        byte = (byte<<1) | bit; 
    }


    mGpioSCL->Set(0);           // start ack cycle
    us_sleep( mUsHold );

    mGpioSDA->SetDirInput(0);   // prep for ack cycle where sda=output
    mGpioSDA->Set( !nack );     // set ack value
    us_sleep( mUsHold );

    mGpioSCL->Set(1);           // start high of ack cycle
    us_sleep( mUsHold );

    mGpioSCL->Set(0);           // end ack cycle
    us_sleep( mUsHold );

    mGpioSDA->SetDirInput(0);   // return SDA to output
    mGpioSDA->Set( 1 );
    us_sleep( mUsHold );

    mGpioSCL->Set(1);           // return SCL to high
    us_sleep( mUsHold );

    *bytePtr = byte;
    return( err );
}

