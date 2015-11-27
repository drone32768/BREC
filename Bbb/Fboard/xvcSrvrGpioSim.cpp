#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "xvcSrvrGpio.h"

enum
{
	test_logic_reset, run_test_idle,

	select_dr_scan, capture_dr, shift_dr,
	exit1_dr, pause_dr, exit2_dr, update_dr,

	select_ir_scan, capture_ir, shift_ir,
	exit1_ir, pause_ir, exit2_ir, update_ir,

	num_states
};

static const int next_state[num_states][2] =
{
	/* [test_logic_reset] =*/ {run_test_idle, test_logic_reset},
	/* [run_test_idle] =   */ {run_test_idle, select_dr_scan},

	/* [select_dr_scan] = */ {capture_dr, select_ir_scan},
	/* [capture_dr] =     */ {shift_dr, exit1_dr},
	/* [shift_dr] =       */ {shift_dr, exit1_dr},
	/* [exit1_dr] =       */ {pause_dr, update_dr},
	/* [pause_dr] =       */ {pause_dr, exit2_dr},
	/* [exit2_dr] =       */ {shift_dr, update_dr},
	/* [update_dr] =      */ {run_test_idle, select_dr_scan},

	/* [select_ir_scan] = */ {capture_ir, test_logic_reset},
	/* [capture_ir] =     */ {shift_ir, exit1_ir},
	/* [shift_ir] =       */ {shift_ir, exit1_ir},
	/* [exit1_ir] =       */ {pause_ir, update_ir},
	/* [pause_ir] =       */ {pause_ir, exit2_ir},
	/* [exit2_ir] =       */ {shift_ir, update_ir},
	/* [update_ir] =      */ {run_test_idle, select_dr_scan}
};

const char *jtag_state_str( int state )
{
    switch( state ){
       case  0 : return( "test_logic_reset" );
       case  1 : return( "run_test_idle" );

       case  2 : return( "select_dr_scan" );
       case  3 : return( "capture_dr" );
       case  4 : return( "shift_dr" );
       case  5 : return( "exit1_dr" );
       case  6 : return( "pause_dr" );
       case  7 : return( "exit2_dr" );
       case  8 : return( "update_dr" );

       case  9 : return( "select_ir_scan" );
       case 10 : return( "capture_ir" );
       case 11 : return( "shift_ir" );
       case 12 : return( "exit1_ir" );
       case 13 : return( "pause_ir" );
       case 14 : return( "exit2_ir" );
       case 15 : return( "update_ir" );
    }
    return("unknown");
}

// Spartan BSCAN JTAG instructions
const char *
inst_str( int inst )
{
   switch( inst ){
       case 0x01: return( "SAMPLE" );
       case 0x02: return( "USER1" );
       case 0x03: return( "USER2" );
       case 0x04: return( "CFG_OUT" );
       case 0x05: return( "CFG_IN" );
       case 0x06: return( "6??" );
       case 0x07: return( "INTEST" );
       case 0x08: return( "USERCODE" );
       case 0x09: return( "IDCODE" );

       case 0x0B: return( "JPROGRAM" );
       case 0x0C: return( "JSTART" );
       case 0x3F: return( "BYPASS" );
   }
   return("undecoded");
}

////////////////////////////////////////////////////////////////////////////////
unsigned int devId = 0x04001093; //  0000 0100 0000 0000 0001 0000 1001 0011
unsigned int devIr = 0x0;
unsigned int devDr = 0x0;
int          devIsBypass = 0;
int devState = test_logic_reset;
int devTdi = 0;
int devTdo = 0;
int devTck = 0;
int devTms = 0;

int devSelectIr = 0;

void DevClk()
{
   switch( devState ){
       case select_dr_scan:
            devSelectIr = 0;
            break;
       case select_ir_scan:
            devSelectIr = 1;
            break;
       case capture_dr:
            // nothing?
            break;
       case capture_ir:
            devIr    = 0x3d; // 0x1;
            break;
       case shift_dr:
            devDr  = devDr >> 1;
            if( devIsBypass ){
               devDr  = devDr | (devTdi?0x2:0);
            }
            else{
               devDr  = devDr | (devTdi?0x80000000:0);
            }
            break;
       case shift_ir:
            devIr  = devIr >> 1;
            devIr  = devIr | (devTdi?0x20:0);
            devIr  = devIr & 0x3f;
            break;
       case update_dr:
            // if( verbose ) printf("-------------->Update DR 0x%08x\n",devDr);
            printf("-------------->Update DR 0x%08x\n",devDr);
            break;
       case update_ir:
            // if( verbose ) printf("-------------->Update IR 0x%02x\n",devIr);
            printf("-------------->Update IR %s 0x%02x\n",
                  inst_str(devIr), devIr);
            devIsBypass = 0;
            if( 0x3C == devIr ){
               devIsBypass = 1;
               devDr       = 0;
            }
            else if( 0x09 == devIr ){
               devDr       = devId;
            }
   }

   if( devSelectIr ){ devTdo = devIr&0x1; }
   else             { devTdo = devDr&0x1; }
   devState = next_state[devState][devTms];

   if( verbose ){
       printf("devState %d (%s)\n", devState, jtag_state_str(devState));
   }
}

////////////////////////////////////////////////////////////////////////////////
void gpio_init(void)
{
   printf("GPIO_INIT\n");
}

void gpio_close(void)
{
   printf("GPIO_CLOSE\n");
}

void gpio_set(int i, int val)
{
   switch( i ){
      case GPIO_TDI: devTdi = val; break;
      case GPIO_TDO: devTdo = val; break;
      case GPIO_TCK: devTck = val; if(1==devTck) DevClk(); break;
      case GPIO_TMS: devTms = val; break;
   }
}

int gpio_get(int i)
{
   int bit;

   switch( i ){
      case GPIO_TDI: bit = devTdi; break;
      case GPIO_TDO: bit = devTdo; break;
      case GPIO_TCK: bit = devTck; break;
      case GPIO_TMS: bit = devTms; break;
   }
   return(bit);
}

