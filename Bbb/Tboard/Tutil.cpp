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

#include "ui2c.h"
#include "mcp4725.h"
#include "max2112.h"

////////////////////////////////////////////////////////////////////////////////
void usage( int exit_code )
{
    printf("-select <N>   select <N>=port[0..2] for subsequent operations\n");
    printf("-enable <EN>  set enable on select port to <EN>=[0..1]\n");
    printf("-init         initial i2c interface\n");
    printf("-usleep <M>   sleeps script execution for <M> micro seconds\n");
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
    MCP4725        dac;
    MAX2112        tuner;
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

        else if( 0==strcmp(argv[idx], "-usleep") ){
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);
            us_sleep( val );
        } 

        else if( 0==strcmp(argv[idx], "-echo") ){
            if( (idx+1) >= argc ){ usage(-1); }
            printf("%s\n",argv[idx+1]);
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

        else if( 0==strcmp(argv[idx], "-init") ){

            // IO1 = p7 = "ss2"   = SCL
            // IO2 = p8 = "stat"  = SDA
            err = ui2c.configure( g6pg->GetSs2(), g6pg->GetStat() );
            printf("ui2c  configure err=0x%08x\n",err);

            err = dac.Configure( &ui2c );
            printf("dac   configure err=0x%08x\n",err);

            err = tuner.Configure( &ui2c );
            printf("tuner configure err=0x%08x\n",err);
        } 

        else if( 0==strcmp(argv[idx], "-read") ){

            if( !g6pg ) { usage(-1); }
            if( (idx+1) >= argc ){ usage(-1); }

            uint8_t bytes[32];
            uint8_t devAddr = strtol(argv[idx+1],&end,0);
            uint8_t regAddr = strtol(argv[idx+2],&end,0);

            printf("I2C Reading dev=0x%02x, reg=0x%02x\n",devAddr, regAddr);
            err =  dac.Read( devAddr, regAddr, bytes, 1 );

            printf("err=0x%08x, value=0x%02x\n",err,bytes[0]);

            break;
        }

        else if( 0==strcmp(argv[idx], "-dac_write") ){
            if( (idx+1) >= argc ){ usage(-1); }

            val = strtol(argv[idx+1],&end,0);
            printf("Set dac to 0x%04x\n",val);

            err = dac.Set( 0xC6, val );
        }

        // Move to next command
        idx++;
    }

    printf("Ttst: Enter Exit\n");
}

