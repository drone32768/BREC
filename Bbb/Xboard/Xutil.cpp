#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"
#include "../Iboard/Iboard.h"

void usage( int exit_code )
{
    printf("-select <N>  select <N>=port[0..2] for subsequent operations\n");
    printf("-enable <EN> set enable on select port to <EN>=[0..1]\n");
    printf("-usleep <M>  sleeps script execution for <M> micro seconds\n");
    printf("-us     <M>  us <M> between clock transitions\n");
    printf("-init        set initial SPI signal\n");
    printf("-write  <N>  write the 16 bit word N\n");
    printf("-echo   <str> echo string <str>\n");
    printf("-seq         sequential read test\n");
    printf("\n");
    printf("NOTE: You must select a port as the first operation\n");
    printf("e.g. -select 0 -enable 1 -write 0x00001\n");
    printf("will select port 0, enable it, and write 1 to it\n");
    exit( exit_code );
}

int usHold = 1000;
int xspiDbg = 0;

int
xspi_init( Gpio6PinGroup *g6pg )
{
    g6pg->GetSs1()->Set( 1 );
    us_sleep( usHold );
    g6pg->GetSclk()->Set( 0 );
    us_sleep( usHold );

    return(0);
}

int 
xspi_write( Gpio6PinGroup *g6pg, int wval )
{
    int rval;
    int obit,ibit,idx;

    if( xspiDbg ){
       printf("xspi_write: wval = 0x%04x\n",wval);
    }

    rval    = 0;

    // expecting: sclk=0, ss=1
    g6pg->GetSs1()->Set( 0 );
    us_sleep( usHold );

    for( idx=15; idx>=0; idx-- ){
        if( xspiDbg ){
            printf("idx[%d]\n",idx);
        }

        if( wval&0x8000 ) obit = 1;
        else              obit = 0;

        if( xspiDbg ){
            printf("   obit = %d\n",obit);
        }
        g6pg->GetMoSi()->Set( obit );
        us_sleep( usHold );
        g6pg->GetSclk()->Set( 1 );
        us_sleep( usHold );
        ibit = g6pg->GetMiSo()->Get( );
        if( xspiDbg ){
            printf("   ibit = %d\n",ibit);
        }

        us_sleep( usHold );
        g6pg->GetSclk()->Set( 0 );
        us_sleep( usHold );

        rval = (rval<<1) | ibit;
        wval = (wval<<1);
    }

    // expecting: sclk=0, ss=0

    g6pg->GetSs1()->Set( 1 );
    us_sleep( usHold );

    if( xspiDbg ){
        printf("xspi_write: rval = 0x%04x\n",rval);
    }
    return(rval);
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

    unsigned short rd;

    printf("Xutil: Enter Main\n");

    portN = 0;
    g6pg  = NULL;

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

        else if( 0==strcmp(argv[idx], "-us") ){
            if( (idx+1) >= argc ){ usage(-1); }
            usHold = strtol(argv[idx+1],&end,0);
        } 

        else if( 0==strcmp(argv[idx], "-echo") ){
            if( (idx+1) >= argc ){ usage(-1); }
            printf("%s\n",argv[idx+1]);
        } 

        else if( 0==strcmp(argv[idx], "-init") ){
            xspi_init( g6pg );
        } 

        else if( 0==strcmp(argv[idx], "-write") ){

            if( !g6pg ) { usage(-1); }
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);
            
            rd = xspi_write( g6pg, val );
            printf("w=0x%04x r=0x%04x (%d)\n",val, rd, rd);

            idx+=2;
            continue;
        }

        else if( 0==strcmp(argv[idx], "-seq") ){
            unsigned short ex;

            int rdCount;
            int erCount;

            rdCount =0;
            erCount =0;
            ex      =0;
            while( 1 ){
                rdCount++;
                if( (rdCount%1000)==0 ){
                   printf("read count %d\n",rdCount);
                }
                rd = xspi_write( g6pg, 0x0001 );

                if( rd!=ex ){
                   printf(
                     "  ERR: %d read 0x%04x, expected 0x%04x total errs=%d\n",
                                     rdCount,rd,ex,erCount);
                   ex = rd;
                   erCount++;
                }
                ex = ex+1;
            }
        }

        idx++;
    }


    printf("Xutil: Enter Exit\n");
}

