#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "xvcSrvrGpio.h"
#include "../Util/gpioutil.h"

GpioUtil gpios[ 4 ];
GpioUtil progBgpio;
GpioUtil doneGpio;
GpioUtil initBgpio;

void gpio_init(void)
{
   printf("GPIO_INIT\n");

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

void gpio_close(void)
{
   printf("GPIO_CLOSE\n");

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

const char * jtag_to_str( int i )
{
   switch( i ){
      case GPIO_TDI: return("tdi");
      case GPIO_TDO: return("tdo");
      case GPIO_TCK: return("tck");
      case GPIO_TMS: return("tms");
   }
}

static int clock_count = 0;
void gpio_set(int i, int val)
{

   if( i == GPIO_TCK ){
      clock_count++;
      if( 0==(clock_count%10000) ){
          printf("clk %d\r",clock_count);
      }
   }

   // printf("set %s = %d\n",jtag_to_str(i),val);
   gpios[i].Set( val );
}

int gpio_get(int i)
{
   int val;
   val = gpios[i].Get();
   // printf("get %s = %d\n",jtag_to_str(i),val);

   return( val );
}

