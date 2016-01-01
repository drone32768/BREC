/*
 * See the comments in header file on the motivation behind this approach
 */

// These are the files from xapp058 and must be resident on build machine 
#include "ports.h"
#include "lenval.h"
#include "micro.h"

// Ugh... yes we are directly including source code.
// As this is a library it can be built one time on the build machine
// These are the files from xapp058 with slight modifications to link
// into this framework
#include "micro.cpp"
#include "lenval.cpp"

#include "jtag_bs.h"
#include "xsvfLib.h"

// See xapp058
void setPort(short p,short val)
{
    switch( p ){
       case TMS: jtag_bs_set_tms(val);
       case TDI: jtag_bs_set_tdi(val);
       case TCK: jtag_bs_set_tck(val);
       default:  printf("setPort unknown jtag signal%d\n",p);
    }
}

// See xapp058
void pulseClock()
{
    setPort(TCK,0);  /* set the TCK port to low  */
    setPort(TCK,1);  /* set the TCK port to high */
}

// See xapp058
void readByte(unsigned char *data)
{
    *data   = xsvf_next_byte();
}

// See xapp058
unsigned char readTDOBit()
{
    return( jtag_bs_get_tdo() );
}

// See xapp058
void waitTime(long microsec)
{
    long        i;
    for ( i = 0; i < microsec; ++i )
    {
        pulseClock();
    }
}

