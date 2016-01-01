#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "../JtagTools/jtag_bs.h"
#include "../Util/mcf.h"
#include "../Util/gpioutil.h"

#define GPIO_TDI 0
#define GPIO_TDO 1
#define GPIO_TCK 2
#define GPIO_TMS 3

GpioUtil gpios[ 4 ];
GpioUtil progBgpio;
GpioUtil doneGpio;
GpioUtil initBgpio;

void jtag_bs_open(void)
{
   printf("JTAG_BS_OPEN(F Board)\n");

   progBgpio.Define( 50 /* gpio1_18 */ );
   progBgpio.Export();
   progBgpio.SetDirInput( 0 );
   progBgpio.Open( );

   doneGpio.Define( 31 /* gpio0_31 */ );
   doneGpio.Export();
   doneGpio.SetDirInput( 1 );
   doneGpio.Open( );

   initBgpio.Define( 30 /* gpio0_30 */ );
   initBgpio.Export();
   initBgpio.SetDirInput( 1 );
   initBgpio.Open( );

   gpios[ GPIO_TDI ].Define( 12 /* gpio0_12 */ );
   gpios[ GPIO_TDI ].Export();
   gpios[ GPIO_TDI ].SetDirInput( 0 );
   gpios[ GPIO_TDI ].Open();

   gpios[ GPIO_TDO ].Define( 51 /* gpio1_19 */ );
   gpios[ GPIO_TDO ].Export();
   gpios[ GPIO_TDO ].SetDirInput( 1 );
   gpios[ GPIO_TDO ].Open();

   gpios[ GPIO_TCK ].Define( 5 /* gpio0_5 */ );
   gpios[ GPIO_TCK ].Export();
   gpios[ GPIO_TCK ].SetDirInput( 0 );
   gpios[ GPIO_TCK ].Open();

   gpios[ GPIO_TMS ].Define( 4 /* gpio0_4 */ );
   gpios[ GPIO_TMS ].Export();
   gpios[ GPIO_TMS ].SetDirInput( 0 );
   gpios[ GPIO_TMS ].Open();
   
   // de-assert program_b
   progBgpio.Set(1);

   printf("done      = %d\n",doneGpio.Get() );
   printf("init_b    = %d\n",initBgpio.Get() );
}

void jtag_bs_close(void)
{
   printf("JTAG_BS_CLOSE(F Board)\n");

   printf("done      = %d\n",doneGpio.Get() );
   printf("init_b    = %d\n",initBgpio.Get() );

   progBgpio.Close();
   doneGpio.Close();
   initBgpio.Close();
   gpios[ GPIO_TDI ].Close();
   gpios[ GPIO_TDO ].Close();
   gpios[ GPIO_TCK ].Close();
   gpios[ GPIO_TMS ].Close();
}

void jtag_bs_set_tms( int v )
{
    gpios[GPIO_TMS].Set(v);
}

void jtag_bs_set_tdi( int v )
{
    gpios[GPIO_TDI].Set(v);
}

void jtag_bs_set_tck( int v )
{
    static unsigned int clock_count = 0;

    clock_count++;
    if( 0==(clock_count%10000) ){
      printf("JTAG_BS CLK %d\n",clock_count);
    }
    gpios[GPIO_TCK].Set(v);
}

unsigned char jtag_bs_get_tdo()
{
   return( gpios[GPIO_TDO].Get() );
}

