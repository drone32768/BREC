#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Rboard.h"

void usage()
{
    printf("-raw <N> where N=16 bit number with 0x prefix for hex\n");
}

int
main( int argc, char *argv[] )
{
    Rboard  *rbrd;
    int      idx;

    printf("Rutil: Enter Main\n");

    rbrd = new Rboard();
    rbrd->SetDev( 0 );
    rbrd->Open();

    idx = 1;
    while( idx < argc ){
        if( 0==strcmp(argv[idx], "-raw") ){
            uint16_t word;
            char     *end;

            if( (idx+1) >= argc ){
               usage();
               exit(-1);
            }
            word = strtol(argv[idx+1],&end,0);
            printf("raw word=0x%04x\n",word);
            rbrd->WriteRaw( word );
            idx+=2;
            continue;
        }

        if( 0==strcmp(argv[idx], "-ch") ){
            uint16_t word;
            char     *end;

            if( (idx+1) >= argc ){
               usage();
               exit(-1);
            }
            word = strtol(argv[idx+1],&end,0);
            printf("ch=0x%04x\n",word);
            rbrd->SetChannel( word );
            idx+=2;
            continue;
        }

        if( 0==strcmp(argv[idx], "-atten") ){
            uint16_t word;
            char     *end;

            if( (idx+1) >= argc ){
               usage();
               exit(-1);
            }
            word = strtol(argv[idx+1],&end,0);
            printf("atten=0x%04x\n",word);
            rbrd->SetAtten( word );
            idx+=2;
            continue;
        }

        if( 0==strcmp(argv[idx], "-w") ){
            uint16_t word;
            char     *end;

            if( (idx+1) >= argc ){
               usage();
               exit(-1);
            }
            word = strtol(argv[idx+1],&end,0);
            printf("w=0x%04x\n",word);
            rbrd->SetW( word );
            idx+=2;
            continue;
        }
    }

    printf("Rutil: Enter Exit\n");
}

