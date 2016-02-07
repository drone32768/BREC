#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"
#include "Iboard.h"

void usage( int exit_code )
{
    printf("-select <N>  select <N>=port[0..2] for subsequent operations\n");
    printf("-enable <EN> set enable on select port to <EN>=[0..1]\n");
    printf("-usleep <M>  sleeps <M> micro seconds\n");
    printf("-mosi   <V>  set mosi on select port to <V>=[0..1]\n");
    printf("-sclk   <V>  set sclk on select port to <V>=[0..1]\n");
    printf("-ss1    <V>  set ss1  on select port to <V>=[0..1]\n");
    printf("-ss2    <V>  set ss2  on select port to <V>=[0..1]\n");
    printf("-miso        get miso on select port\n");
    printf("-stat        get stat on select port\n");
    printf("\n");
    printf("NOTE: You must select a port as the first operation\n");
    printf("e.g. -select 0 -enable 1 -mosi 1\n");
    printf("will select port 0, enable it, and set the mosi to 1\n");
    exit( exit_code );
}

int
main( int argc, char *argv[] )
{
    Iboard  *ibrd;
    int      idx;

    Gpio6PinGroup *g6pg;
    int            portN;

    int      val;
    char     *end;

    printf("Iutil: Enter Main\n");
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

        else if( 0==strcmp(argv[idx], "-mosi") ){
            if( !g6pg ) { usage(-1); }
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);

            printf("mosi port=%d set=%d\n",portN,val);
            g6pg->GetMoSi()->Set( val );

            idx+=2;
            continue;
        }

        else if( 0==strcmp(argv[idx], "-sclk") ){
            if( !g6pg ) { usage(-1); }
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);

            printf("sclk port=%d set=%d\n",portN,val);
            g6pg->GetSclk()->Set( val );

            idx+=2;
            continue;
        }

        else if( 0==strcmp(argv[idx], "-ss1") ){
            if( !g6pg ) { usage(-1); }
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);

            printf("ss1 port=%d set=%d\n",portN,val);
            g6pg->GetSs1()->Set( val );

            idx+=2;
            continue;
        }

        else if( 0==strcmp(argv[idx], "-ss2") ){
            if( !g6pg ) { usage(-1); }
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);

            printf("ss2 port=%d set=%d\n",portN,val);
            g6pg->GetSs2()->Set( val );

            idx+=2;
            continue;
        }

        else if( 0==strcmp(argv[idx], "-miso") ){
            if( !g6pg ) { usage(-1); }

            val = g6pg->GetMiSo()->Get( );
            printf("miso port=%d get=%d\n",portN,val);

            idx+=1;
            continue;
        }

        else if( 0==strcmp(argv[idx], "-stat") ){
            if( !g6pg ) { usage(-1); }

            val = g6pg->GetStat()->Get( );
            printf("stat port=%d get=%d\n",portN,val);

            idx+=1;
            continue;
        }

        idx++;
    }

    printf("Iutil: Enter Exit\n");
}

