#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"

GpioUtil progBgpio;
GpioUtil doneGpio;
GpioUtil initBgpio;

void usage( int exit_code )
{
    printf("-prog        set program_b low\n");
    printf("-release     set program_b high\n");
    printf("-show        display state of done/init_b\n");
    printf("-echo <str>  echo <str>\n");
    printf("-usleep <M>  sleeps execution for <M> microseconds\n");
    exit( exit_code );
}

int
main( int argc, char *argv[] )
{
    int            idx;
    int            val;
    char          *end;

    printf("Fctl: Enter Main\n");

    progBgpio.Define( 50 /* gpio1_18 */ );
    progBgpio.Export();
    progBgpio.SetDirInput( 0 );
    progBgpio.Open();

    doneGpio.Define( 31 /* gpio0_31 */ );
    doneGpio.Export();
    doneGpio.SetDirInput( 1 );
    doneGpio.Open();

    initBgpio.Define( 30 /* gpio0_30 */ );
    initBgpio.Export();
    initBgpio.SetDirInput( 1 );
    initBgpio.Open();

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

        else if( 0==strcmp(argv[idx], "-prog") ){
           progBgpio.Set(0);
        }

        else if( 0==strcmp(argv[idx], "-release") ){
           progBgpio.Set(1);
        }

        else if( 0==strcmp(argv[idx], "-show") ){
           printf("done      = %d\n",doneGpio.Get() );
           printf("init_b    = %d\n",initBgpio.Get() );
           printf("program_b = %d\n",progBgpio.Get() );
        }

        idx++;
    }


    printf("Fctl: Exit\n");
}
