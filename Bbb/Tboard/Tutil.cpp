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

    printf("%s: Enter Main\n",__FILE__);

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

            printf("%s:enable port=%d enable=%d\n",__FILE__,portN,val);
            ibrd->EnablePort( portN, val );

            idx+=2;
            continue;
        }

        else if( 0==strcmp(argv[idx], "-init") ){

            // IO1 = p7 = "ss2"   = SCL
            // IO2 = p8 = "stat"  = SDA
            err = ui2c.configure( g6pg->GetSs2(), g6pg->GetStat() );
            printf("%s:ui2c  configure err=0x%08x\n",__FILE__,err);

            err = dac.Configure( &ui2c );
            printf("%s:dac   configure err=0x%08x\n",__FILE__,err);

            err = tuner.Configure( &ui2c );
            printf("%s:tuner configure err=0x%08x\n",__FILE__,err);
        } 

        else if( 0==strcmp(argv[idx], "-dac_write") ){
            if( (idx+1) >= argc ){ usage(-1); }

            val = strtol(argv[idx+1],&end,0);

            err = dac.Set( 0xC6, val );
            printf("%s:dac_write val = 0x%04x err = 0x%08x\n",__FILE__,val,err);
        }

        else if( 0==strcmp(argv[idx], "-dac_read") ){
            err = dac.Get( 0xC6, &val );
            printf("%s:dac_read  val = 0x%04x err = 0x%08x\n",__FILE__,val,err);
        }

        else if( 0==strcmp(argv[idx], "-dac_mod") ){
            int up = 1;
            val = 0x0;
            while( 1 ){
                dac.Set( 0xC6, val  );
                if( up ){ val = (val+32); }
                else    { val = (val-32); }
                if( val > 0xfff ) { up=0; val=0xfff; }
                if( val < 0     ) { up=1; val=0x0; }
                
            }
        }

        // Move to next command
        idx++;
    }

    printf("Ttst: Enter Exit\n");
}

