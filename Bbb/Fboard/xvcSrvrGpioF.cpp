#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "xvcSrvrGpio.h"
#include "../Util/gpioutil.h"

GpioUtil gpios[ 4 ];

void gpio_init(void)
{
   if( verbose ) printf("GPIO_INIT\n");

   gpios[ GPIO_TDI ].Define( 12 /* gpio0_12 */ );
   gpios[ GPIO_TDI ].Export();
   gpios[ GPIO_TDI ].SetDirInput( 1 );
   gpios[ GPIO_TDI ].Open();

   gpios[ GPIO_TDO ].Define( 51 /* gpio1_19 */ );
   gpios[ GPIO_TDO ].Export();
   gpios[ GPIO_TDO ].SetDirInput( 0 );
   gpios[ GPIO_TDO ].Open();

   gpios[ GPIO_TCK ].Define( 5 /* gpio0_5 */ );
   gpios[ GPIO_TCK ].Export();
   gpios[ GPIO_TCK ].SetDirInput( 0 );
   gpios[ GPIO_TCK ].Open();

   gpios[ GPIO_TMS ].Define( 4 /* gpio9_4 */ );
   gpios[ GPIO_TMS ].Export();
   gpios[ GPIO_TMS ].SetDirInput( 0 );
   gpios[ GPIO_TMS ].Open();
   
}

void gpio_close(void)
{
   if( verbose ) printf("GPIO_CLOSE\n");
   gpios[ GPIO_TDI ].Close();
   gpios[ GPIO_TDO ].Close();
   gpios[ GPIO_TCK ].Close();
   gpios[ GPIO_TMS ].Close();
}

void gpio_set(int i, int val)
{
   gpios[i].Set( val );
}

int gpio_get(int i)
{
   return( gpios[i].Get() );
}

