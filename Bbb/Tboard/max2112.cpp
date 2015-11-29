
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"
#include "ui2c.h"

#include "max2112.h"

////////////////////////////////////////////////////////////////////////////////
MAX2112::MAX2112()
{
   mDbg = 0x0;
}

////////////////////////////////////////////////////////////////////////////////
uint32_t 
MAX2112::Configure( UI2C *ui2c )
{
   mI2c = ui2c;
   return( 0 );
}

