#include <stdio.h>
#include <stdlib.h>
#include <string.h>


FILE *inputFh;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include "../Util/mcf.h"
#include "../Util/gpioutil.h"
#include "../JtagTools/xsvfLib.h"
#include "../JtagTools/jtag_bs.h"

unsigned char xsvf_next_byte()
{
   return( (unsigned char)fgetc(inputFh) );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] )
{
    int     err   = 0;
    char*   fName = 0;
    int     i;

    for ( i = 1; i < argc ; ++i )
    {
        if ( 0==strcmp( argv[ i ], "-v" ) )
        {
            ++i;
            if ( i >= argc )
            {
                printf( "ERROR:  missing <level> parameter for -v option.\n" );
            }
            else
            {
                xsvf_iDebugLevel    = atoi( argv[ i ] );
                printf( "Verbose level = %d\n", xsvf_iDebugLevel );
            }
        }
        else
        {
            fName  = argv[ i ];
        }
    }

    if ( !fName )
    {
        printf( "usage [-v level] file\n" );
        printf( " -v level   verbose, level = 0-4 (default=0)\n" );
        printf( " file       xsvf file to execute\n" );
        return( -254 );
    }
    printf( "Loading file = %s\n", fName );

    jtag_bs_open();
    jtag_bs_set_tms(1);

    inputFh = fopen( fName, "rb" );
    if ( !inputFh )
    {
        fprintf(stderr,"ERROR:  Cannot open file %s\n", fName );
        return( -255 );
    }
    err  = xsvfExecute();
    fclose( inputFh );

    printf( "Done w/ err = %d\n", err );

    return( err );
}
