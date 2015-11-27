/*
 * This is a simple stand alone utility to exercize a T board I2C interface.
 *
 * All communication is done using the gpio's on an I board port and
 * contained within this file.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"
#include "../Iboard/Iboard.h"

////////////////////////////////////////////////////////////////////////////////
/*
I2C items to remember:
 a) Data can only change while scl is low

 b) SDA and SCL remain high while the bus is not busy.
    The pullups on both maintain this.
    [ Max2112 p14 ]

Questions:
 1.0 What do we leave SCL at on exit (what state is it left in - hi/lo)
 2.0 What do we leave SDA at on exit (input/output)
 3.0 On changing SDA in/out direction does this cause a state change
     that could be misinterpreted
 4.0 How to avoid multiple entities driving SDA

*/

class UI2C {

#   define UI2C_ERR_CFG       0x00000001
#   define UI2C_ERR_WRITE_ACK 0x00000002
#   define UI2C_ERR_READ_ACK  0x00000004

public:

    UI2C();

    uint32_t Cfg(  GpioUtil *scl, GpioUtil *sda );

    uint32_t Write( uint8_t  devAddr, 
                    uint8_t  regAddr, 
                    uint8_t *regBytes, 
                    int      nBytes );

    uint32_t Read(  uint8_t  devAddr, 
                    uint8_t  regAddr, 
                    uint8_t *regBytes, 
                    int      nBytes );
private:

    uint32_t  start();
    uint32_t  stop();
    uint32_t  write_cycle( uint8_t byte );
    uint32_t  read_cycle(  uint8_t *bytePtr, int nack );

    int       mUsHold;
    int       mDbg;
    GpioUtil *mGpioSCL;
    GpioUtil *mGpioSDA;
};

////////////////////////////////////////////////////////////////////////////////
UI2C::UI2C()
{
    mUsHold   = 100;
    mDbg      = 0;
    mGpioSCL  = NULL;
    mGpioSDA  = NULL;
}

////////////////////////////////////////////////////////////////////////////////
uint32_t
UI2C::Cfg( GpioUtil *scl, GpioUtil *sda )
{
    // Save the two pins
    mGpioSCL = scl;
    mGpioSDA = sda;
 
    // Sanity check on init
    if( !mGpioSCL || !mGpioSDA ){
        printf("%s:%d gpios null\n",__FILE__,__LINE__);
        return( UI2C_ERR_CFG );
    }

    // Initialize clock pin state
    mGpioSCL->SetDirInput(0);
    mGpioSCL->Set(0);

    // Initialize data pin state
    mGpioSDA->SetDirInput(0);
    mGpioSDA->Set(1);

    // Wait for signals to hold 
    us_sleep( mUsHold );

    return(0);
}

////////////////////////////////////////////////////////////////////////////////
//
// START = SDA high -> low while SCL is high
// Pre : SCL=high  SDA=???/high
// Post: SCL=high  SDA=out/low
//
uint32_t
UI2C::start()
{
    mGpioSDA->SetDirInput(0);
    mGpioSDA->Set(1);
   
    mGpioSCL->Set(1);
    us_sleep( mUsHold );

    mGpioSDA->Set(0);
    us_sleep( mUsHold );

    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
//
// STOP = SDA low->high while SCL is high
// Pre : SCL=high  SDA=???/??? 
// Post: SCL=high  SDA=out/high
//
uint32_t
UI2C::stop()
{
   
    mGpioSCL->Set(1);
    us_sleep( mUsHold );

    // if by error sda were high, this looks like a start
    // however, the next transition is a stop
    mGpioSDA->SetDirInput(0); 
    mGpioSDA->Set(0);
    us_sleep( mUsHold );

    mGpioSDA->Set(1);
    us_sleep( mUsHold );

    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
// Pre : SCL=high  SDA=out/??? 
// Post: SCL=high  SDA=out/high
uint32_t
UI2C::write_cycle( uint8_t byte )
{
    int           err = 0;
    int           cnt;
    int           ab;

    for(cnt=0;cnt<8;cnt++){

        mGpioSCL->Set(0);
        us_sleep( mUsHold );

        mGpioSDA->Set( ((byte&0x80)>>7) );
        us_sleep( mUsHold );

        mGpioSCL->Set(1);
        us_sleep( mUsHold );

        byte = byte << 1;
    }

    // SCL = high
    // SDA = output

    mGpioSDA->SetDirInput(1);   // prep for ack cycle where sda=input

    mGpioSCL->Set(0);           // start low of ack cycle
    us_sleep( mUsHold );
    mGpioSCL->Set(1);           // start high of ack cycle
    us_sleep( mUsHold );

    ab = mGpioSDA->Get();       // get ack from sda
    if( !ab ){
        err |= UI2C_ERR_WRITE_ACK;
    }

    mGpioSCL->Set(0);           // scl low to rest sda 
    us_sleep( mUsHold );

    mGpioSDA->SetDirInput(0);   // return SDA to output
    mGpioSDA->Set( 1 );
    us_sleep( mUsHold );

    mGpioSCL->Set(1);           // return SCL to high

    return( err );
}

////////////////////////////////////////////////////////////////////////////////
// Pre : SCL=high  SDA=out/??? 
// Post: SCL=high  SDA=out/high
uint32_t
UI2C::read_cycle( uint8_t *bytePtr, int nack )
{
    int           err = 0;
    int           cnt;
    int           bit;
    uint8_t       byte = 0;

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

    // SCL = high
    // SDA = input

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

////////////////////////////////////////////////////////////////////////////////
uint32_t 
UI2C::Write( uint8_t devAddr, uint8_t regAddr, uint8_t *regBytes, int nBytes )
{
    int           err = 0;
    int           cerr;
    int           idx;

    cerr = start();
    err |= cerr;
    if( cerr && mDbg ){
        printf("%s:%d: start[1] 0x%08x\n",__FILE__,__LINE__,cerr);
    }

    cerr = write_cycle( (devAddr&0xfe) );
    err |= cerr;
    if( cerr && mDbg ){
        printf("%s:%d: dev addr write cycle 0x%08x\n",__FILE__,__LINE__,cerr);
    }

    cerr = write_cycle( (regAddr&0xff) );
    err |= cerr;
    if( cerr && mDbg ){
        printf("%s:%d: reg addr write cycle 0x%08x\n",__FILE__,__LINE__,cerr);
    }

    for(idx=0;idx<nBytes;idx++){
        cerr = write_cycle( regBytes[idx] );
        err |= cerr;
        if( cerr && mDbg ){
            printf("%s:%d: value write cycle idx=%d/%d 0x%08x\n",
                            __FILE__,__LINE__,cerr,idx,nBytes);
        }
    }

    cerr = stop();
    err |= cerr;
    if( cerr && mDbg ){
        printf("%s:%d: stop 0x%08x\n",__FILE__,__LINE__,cerr);
    }
 
    return( err );
}

////////////////////////////////////////////////////////////////////////////////
uint32_t 
UI2C::Read( uint8_t devAddr, uint8_t regAddr, uint8_t *regBytes, int nBytes )
{
    int           err = 0;
    int           cerr;
    int           idx;

    cerr = start();
    err |= cerr;
    if( cerr && mDbg ){
        printf("%s:%d: start[1] 0x%08x\n",__FILE__,__LINE__,cerr);
    }

    cerr = write_cycle( (devAddr&0xfe) );
    err |= cerr;
    if( cerr && mDbg ){
        printf("%s:%d: dev addr write cycle 0x%08x\n",__FILE__,__LINE__,cerr);
    }

    cerr = write_cycle( (regAddr&0xff) );
    err |= cerr;
    if( cerr && mDbg ){
        printf("%s:%d: reg addr write cycle 0x%08x\n",__FILE__,__LINE__,cerr);
    }

    cerr = start();
    err |= cerr;
    if( cerr && mDbg ){
        printf("%s:%d: start[2] 0x%08x\n",__FILE__,__LINE__,cerr);
    }

    cerr = write_cycle( (devAddr&0xfe) | 1 );
    err |= cerr;
    if( cerr && mDbg ){
        printf("%s:%d: write cycle R=1 0x%08x\n",__FILE__,__LINE__,cerr);
    }

    for(idx=0;idx<nBytes;idx++){
        cerr = read_cycle( &(regBytes[idx]), (idx==(nBytes-1)?1:0) );
        err |= cerr;
        if( cerr && mDbg ){
            printf("%s:%d: read cycle idx=%d/%d 0x%08x\n",
                            __FILE__,__LINE__,cerr,idx,nBytes);
        }
    }

    cerr = stop();
    err |= cerr;
    if( cerr && mDbg ){
        printf("%s:%d: stop 0x%08x\n",__FILE__,__LINE__,cerr);
    }

    return( err );
}

////////////////////////////////////////////////////////////////////////////////
void usage( int exit_code )
{
    printf("-select <N>   select <N>=port[0..2] for subsequent operations\n");
    printf("-enable <EN>  set enable on select port to <EN>=[0..1]\n");
    printf("-usleep <M>   sleeps script execution for <M> micro seconds\n");
    printf("-init         initial i2c interface\n");
    printf("-echo   <str> echo string <str>\n");
    printf("-write  <N>   write the 16 bit word N\n");
    printf("\n");
    printf("NOTE: You must select a port as the first operation and init\n");
    printf("e.g. -select 0 -enable 1 -init -write ...\n");
    exit( exit_code );
}

int
main( int argc, char *argv[] )
{
    Iboard        *ibrd;
    int            idx;

    Gpio6PinGroup *g6pg;
    int            portN;

    int            val;
    char          *end;

    UI2C           ui2c;
    int            err;

    printf("Ttst: Enter Main\n");

    portN   = 0;
    g6pg    = NULL;

    ibrd = new Iboard();
    ibrd->Open();

    idx = 1;
    while( idx < argc ){

        if( 0==strcmp(argv[idx], "-help") ){
            usage(0);
        }

        else if( 0==strcmp(argv[idx], "-h") ){
            usage(0);
        }

        else if( 0==strcmp(argv[idx], "-select") ){
            if( (idx+1) >= argc ){ usage(-1); }

            portN = strtol(argv[idx+1],&end,0);
            g6pg  = ibrd->AllocPort( portN );

            idx+=2;
            continue;
        }

        else if( 0==strcmp(argv[idx], "-enable") ){
            if( !g6pg ) { usage(-1); }
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);

            printf("enable port=%d enable=%d\n",portN,val);
            ibrd->EnablePort( portN, val );

            idx+=2;
            continue;
        }

        else if( 0==strcmp(argv[idx], "-usleep") ){
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);
            us_sleep( val );
        } 

        else if( 0==strcmp(argv[idx], "-echo") ){
            if( (idx+1) >= argc ){ usage(-1); }
            printf("%s\n",argv[idx+1]);
        } 

        else if( 0==strcmp(argv[idx], "-init") ){
            // IO1 = p7 = "stat" = SCL
            // IO2 = p8 = "ss2"  = SDA
            err = ui2c.Cfg( g6pg->GetStat(), g6pg->GetSs2() );
            printf("err=0x%08x\n",err);
        } 

        else if( 0==strcmp(argv[idx], "-write") ){

            if( !g6pg ) { usage(-1); }
            if( (idx+1) >= argc ){ usage(-1); }

            uint8_t bytes[32];
            uint8_t devAddr = strtol(argv[idx+1],&end,0);
            uint8_t regAddr = strtol(argv[idx+2],&end,0);

            bytes[0] = strtol(argv[idx+3],&end,0);

            printf("I2C Writing dev=0x%02x, reg=0x%02x\n",devAddr, regAddr);
            printf("   val=0x%02x\n",bytes[0]);
            err = ui2c.Write( devAddr, regAddr, bytes, 1 );

            printf("err=0x%08x\n",err);

            break;
        }

        else if( 0==strcmp(argv[idx], "-read") ){

            if( !g6pg ) { usage(-1); }
            if( (idx+1) >= argc ){ usage(-1); }

            uint8_t bytes[32];
            uint8_t devAddr = strtol(argv[idx+1],&end,0);
            uint8_t regAddr = strtol(argv[idx+2],&end,0);

            printf("I2C Reading dev=0x%02x, reg=0x%02x\n",devAddr, regAddr);
            err = ui2c.Read( devAddr, regAddr, bytes, 1 );

            printf("err=0x%08x, value=0x%02x\n",err,bytes[0]);

            break;
        }
        // Move to next command
        idx++;
    }

    printf("Ttst: Enter Exit\n");
}

