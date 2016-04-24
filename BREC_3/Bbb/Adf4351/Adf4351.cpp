//
//
// This source code is available under the "Simplified BSD license".
//
// Copyright (c) 2013, J. Kleiner
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, 
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright 
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the original author nor the names of its contributors 
//    may be used to endorse or promote products derived from this software 
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
// OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/time.h>

#include "Util/mcf.h"  // for us_sleep
#include "Util/gpioutil.h"

#include "Adf4351Regs.h"
#include "Adf4351.h"

//----------------------------------------------------------------------------
/**
 * Zero arg constructor.  At a minimum the device number must be set after
 * this but prior to general use.
 */
Adf4351::Adf4351()
{
    mDevNum = -1;
    mIsOpen = 0;
    mLog    = 0x0; 
    //mLog    = 0xffffffff; 

    mR      = 1;
    mInt    = 43680;
    mMod    = 4095;
    mFrac   = 0;
}

//----------------------------------------------------------------------------
static int ShowAdf4351Word( int dev, uint32_t word )
{
    int reg;
    reg = word & 0x00000007;
    printf("  write D=%d R%d = 0x%08x\n",dev,reg,word);
    switch( reg ){
        case 0:{
            printf("    Res         %d\n", (word>>31)&0x1);
            printf("    INT         %d\n", (word>>15)&0xffff);
            printf("    FRAC        %d\n", (word>>3)&0x3ff);
            break;
        }
        case 1:{
            printf("    Res         %d\n", (word>>29)&0x7);
            printf("    PhaseAdj    %d\n", (word>>28)&0x1);
            printf("    Prescaler   %d\n", (word>>27)&0x1);
            printf("    Phase       %d\n", (word>>15)&0x3ff);
            printf("    Mod         %d\n", (word>>3)&0x3ff);
            break;
        }
        case 2:{
            printf("    Res         %d\n", (word>>31)&0x1);
            printf("    NoiseMode   %d\n", (word>>29)&0x3);
            printf("    MuxOut      %d\n", (word>>26)&0x3);
            printf("    RDOUBLER    %d\n", (word>>25)&0x1);
            printf("    RDIV2       %d\n", (word>>24)&0x1);
            printf("    R           %d\n", (word>>14)&0xfff);
            printf("    DoubleBf    %d\n", (word>>13)&0x1);
            printf("    ChrgPumpI   %d\n", (word>>9) &0xf);
            printf("    Ldf         %d\n", (word>>8) &0x1);
            printf("    Ldp         %d\n", (word>>7) &0x1);
            printf("    PdPolarity  %d\n", (word>>6) &0x1);
            printf("    PwrDn       %d\n", (word>>5) &0x1);
            printf("    CpTriState  %d\n", (word>>4) &0x1);
            printf("    CounterRst  %d\n", (word>>3) &0x1);
            break;
        }
        case 3:{
            printf("    Res         %d\n", (word>>24)&0xff);
            printf("    BS          %d\n", (word>>23)&0x1);
            printf("    ABP         %d\n", (word>>22)&0x1);
            printf("    ChargeCan   %d\n", (word>>21)&0x1);
            printf("    Res         %d\n", (word>>19)&0x3);
            printf("    CSR         %d\n", (word>>18)&0x1);
            printf("    CLK_DIV_MD  %d\n", (word>>15)&0x3);
            printf("    CLK_DIV     %d\n", (word>>3)&0xfff);
            break;
        }
        case 4:{
            printf("    Res         %d\n", (word>>24)&0xff);
            printf("    FBselect    %d\n", (word>>23)&0x1);
            printf("    DIV_SEL     %d\n", (word>>20)&0x7);
            printf("    BS_CLK_DIV  %d\n", (word>>12)&0xff);
            printf("    VCO_PDN     %d\n", (word>>11)&0x1);
            printf("    MTLD        %d\n", (word>>10)&0x1);
            printf("    AUX_OUT_SEL %d\n", (word>>9)&0x1);
            printf("    AUX_EN      %d\n", (word>>8)&0x1);
            printf("    AUX_PWR     %d\n", (word>>6)&0x3);
            printf("    RF_EN       %d\n", (word>>5)&0x1);
            printf("    RF_PWR      %d\n", (word>>3)&0x3);
            break;
        }
        case 5:{
            printf("    Res         %d\n", (word>>24)&0xff);
            printf("    LD_PIN_MODE %d\n", (word>>22)&0x3);
            printf("    Res         %d\n", (word>>21)&0x1);
            printf("    Res         %d\n", (word>>19)&0x3);
            printf("    Res         %d\n", (word>>3) &0xffff);
            break;
        }
        default:{
            printf("???\n");
        }
    }
    return(0);
}

//----------------------------------------------------------------------------
void Adf4351::SetLog( unsigned int mask )
{
    mLog = mask;
}

//----------------------------------------------------------------------------
/**
 * Set the device number of this device.  It determines which chip select
 * and lock detect lines are used in all subsequent operations.
 */
void Adf4351::SetDev( int dn )
{
    mDevNum = dn;
}

//----------------------------------------------------------------------------
/**
 * This routine will toggle the lock detector for the device a finite
 * number of times between 0 and 1.  Each toggle the results are checked.
 * If the lock signal is as specified a 0 is returned, else a non zero value.
 * NOTE: Only R5 is access and it is returned to reflect digital lock state.
 *       No internal state of the device or software is modified.
 */
int Adf4351::SpiTest()
{
   int          ld_high, ld_low;	 
   int          cnt;
   unsigned int r5;

   for(cnt=0;cnt<5;cnt++){
       r5                 =  ADF4351_REG5_LD_PIN_MODE_LOW               |
                             0x00180000                                 |
                             0x5;
       (mSpiWriteFunc)(mDevNum,mLog&ADF_LOG_WRITES,r5);
       us_sleep( 100 );
       ld_low = (mGetLockFunc)(mDevNum);

       r5                 =  ADF4351_REG5_LD_PIN_MODE_HIGH              |
	                     0x00180000                                 |
                             0x5;
       (mSpiWriteFunc)(mDevNum,mLog&ADF_LOG_WRITES,r5);
       us_sleep( 100 );
       ld_high = (mGetLockFunc)(mDevNum);

       if( !(1==ld_high) || !(0==ld_low) ){
           fprintf(stderr,
                   "Adf4351::SpiTest spi_test failed d=%d high=%d low=%d\n",
                   mDevNum,ld_high,ld_low);
           return( -1 );
       }	    
       else{
           if( mLog & ADF_LOG_SPI_TESTS ){
           fprintf(stderr,
                   "Adf4351::SpiTest spi_test ok d=%d high=%d low=%d\n",
                   mDevNum,ld_high,ld_low);
           }
       }

   }
   return(0);
}

//----------------------------------------------------------------------------
/**
 * This method tests the spi interface to the device and configures
 * the registers.
 */
int Adf4351::Open( 
    Adf4351SpiWriteFuncPtr spiWriteFunc,
    Adf4351GetLdFuncPtr    getLockFunc )
{
    int err;

    // Save the io interface we are provided
    mSpiWriteFunc = spiWriteFunc;
    mGetLockFunc  = getLockFunc;

    // Test the spi interface and lock detect signal 
    if( mLog & ADF_LOG_SPI_TESTS ){
       printf("Adf4351::Open() SPI Test (LD toggle) #######################\n");
    }
    err = this->SpiTest();
    if( err ){
        fprintf(stderr,"Adf4351::Open SpiTest()  failed dev=%d\n",mDevNum);
    }

    //  Setup the basic configuration
    mRegs[ADF4351_REG5] =  ADF4351_REG5_LD_PIN_MODE_DIGITAL           | 
  	                   0x00180000                                 |
                           0x5;

    mRegs[ADF4351_REG4] =  ADF4351_REG4_FEEDBACK_FUND                 |
	                   ADF4351_REG4_RF_DIV_SEL( 6 )               |
                           ADF4351_REG4_8BIT_BAND_SEL_CLKDIV( 255 )   |
                           ADF4351_REG4_AUX_OUTPUT_DIV                |
                           ADF4351_REG4_AUX_OUTPUT_EN                 |
                           ADF4351_REG4_AUX_OUTPUT_PWR(1)             |
                           ADF4351_REG4_RF_OUT_EN                     |     
                           ADF4351_REG4_OUTPUT_PWR(1)                 |
                           0x4;

    mRegs[ADF4351_REG3] =  ADF4351_REG3_12BIT_CLKDIV(1)               |
                           0x3;

    mRegs[ADF4351_REG2] =  ADF4351_REG2_10BIT_R_CNT( mR   )           |
                           ADF4351_REG2_CHARGE_PUMP_CURR_uA( 512 )    |
                           ADF4351_REG2_PD_POLARITY_POS               |
                           0x2;

    mRegs[ADF4351_REG1] =  ADF4351_REG1_PRESCALER                     |
                           ADF4351_REG1_MOD( mMod )                   | 
                           0x1;

    mRegs[ADF4351_REG0] =  ADF4351_REG0_FRACT( mFrac )                |
                           ADF4351_REG0_INT( mInt  )                  |
                           0x0;

    if( mLog & ADF_LOG_SPI_TESTS ){
       printf("Adf4351::Open Begin Configure ##############################\n");
    }
    (mSpiWriteFunc)(mDevNum,mLog&ADF_LOG_WRITES,mRegs[ADF4351_REG5]);
    (mSpiWriteFunc)(mDevNum,mLog&ADF_LOG_WRITES,mRegs[ADF4351_REG4]);
    (mSpiWriteFunc)(mDevNum,mLog&ADF_LOG_WRITES,mRegs[ADF4351_REG3]);
    (mSpiWriteFunc)(mDevNum,mLog&ADF_LOG_WRITES,mRegs[ADF4351_REG2]);
    (mSpiWriteFunc)(mDevNum,mLog&ADF_LOG_WRITES,mRegs[ADF4351_REG1]);
    (mSpiWriteFunc)(mDevNum,mLog&ADF_LOG_WRITES,mRegs[ADF4351_REG0]);

    if( mLog & ADF_LOG_SPI_TESTS ){
       printf("Adf4351::Open End   Configure ##############################\n");
    }

    mIsOpen = 1;

    return( 0 );
}

//----------------------------------------------------------------------------
/**
 * Returns the lock status of device. 1 = locked, 0 = not locked
 */
int Adf4351::GetLock()
{
    return( (mGetLockFunc)(mDevNum) );
}

//----------------------------------------------------------------------------
/**
 * This method sets the frequency as close the value specified as possible
 * Argument : Requested frequency in Hertz
 * Return   : Actual frequency in Hertz
 */
int64_t Adf4351::SetFrequency( int64_t freqHz )
{
    double Fin;
    double Fpsd;
    double frac;
    double Fvco;
    int    mul,divCode;
    unsigned int prevR4;

    if( mLog & ADF_LOG_FREQ_CALC ){
        printf("Adf4351::SetFrequency d=%d f=%f\n",mDevNum,(double)freqHz );
    }

    if( freqHz/1e6 < 34 ){
        fprintf(stderr,"f=%f too low\n",(double)freqHz);
        return(1);
    }
    if( freqHz/1e6 > 4500 ){
        fprintf(stderr,"f=%f too high\n",(double)freqHz);
        return(1);
    }

    // Reference frequency specified by oscillator on board
    Fin  = 25000000;
    Fpsd = Fin/mR;

    // Fvco must be within 2200Mhz - 4400Mhz
    Fvco = freqHz;
    mul  = 1;
    while( Fvco < 2200e6 ){
       mul  = mul  * 2;
       Fvco = Fvco * 2;
    }
    if( mLog & ADF_LOG_FREQ_CALC ){
        printf("Div=%3d, Fvco=%f Fin=%f Fpsd=%f\n",mul,Fvco,Fin,Fpsd);
    }

    mInt = Fvco / Fpsd;
    frac = ( Fvco - (mInt*Fpsd) )/Fpsd;
    mFrac= frac * mMod;

    if( mLog & ADF_LOG_FREQ_CALC ){
        printf("INT=%8d, FRAC=%8d\n",mInt,mFrac);
    }

    freqHz = (mInt*Fpsd + Fpsd*mFrac/mMod )/mul;
    if( mLog & ADF_LOG_FREQ_CALC ){
        printf("Factual=%f\n",(double)freqHz);
    }

    switch( mul ){
        case  1: divCode = 0;break;
        case  2: divCode = 1;break;
        case  4: divCode = 2;break;
        case  8: divCode = 3;break;
        case 16: divCode = 4;break;
        case 32: divCode = 5;break;
        case 64: divCode = 6;break;
        default: divCode = 6;break;
    }

    // Update R4 for divider and only program if different
    prevR4               =  mRegs[ADF4351_REG4];
    mRegs[ADF4351_REG4] &= ~ADF4351_REG4_RF_DIV_SEL( 0x7 );
    mRegs[ADF4351_REG4] |=  ADF4351_REG4_RF_DIV_SEL( divCode );
    if( mRegs[ADF4351_REG4] !=  prevR4 ){
        (mSpiWriteFunc)(mDevNum,mLog&ADF_LOG_WRITES,mRegs[ADF4351_REG4]);
    }

    // Program R0 for int/frac
    mRegs[ADF4351_REG0]  =  ADF4351_REG0_FRACT( mFrac )                |
                            ADF4351_REG0_INT( mInt  )                  |
                            0x0;
    (mSpiWriteFunc)(mDevNum,mLog&ADF_LOG_WRITES,mRegs[ADF4351_REG0]);

    mFactHz = freqHz;
    return( freqHz );
}

//----------------------------------------------------------------------------
/**
 * This method gets the last actual frequency programmed
 * Argument : None.
 * Return   : Actual frequency in Hertz
 */
int64_t Adf4351::GetFrequency()
{
    return(mFactHz);
}

//----------------------------------------------------------------------------
int Adf4351::SetAuxPower( int pwr )
{
    mRegs[ADF4351_REG4] &= ~ADF4351_REG4_AUX_OUTPUT_PWR( 0x3 );
    mRegs[ADF4351_REG4] |=  ADF4351_REG4_AUX_OUTPUT_PWR( pwr&0x3 );
    (mSpiWriteFunc)(mDevNum,mLog&ADF_LOG_WRITES,mRegs[ADF4351_REG4]);
    return(0);
}

//----------------------------------------------------------------------------
int Adf4351::SetAuxEnable( int enable )
{
    if( enable ){
        mRegs[ADF4351_REG4] |=  ADF4351_REG4_AUX_OUTPUT_EN;
    }
    else{
        mRegs[ADF4351_REG4] &= ~ADF4351_REG4_AUX_OUTPUT_EN;
    }
    (mSpiWriteFunc)(mDevNum,mLog&ADF_LOG_WRITES,mRegs[ADF4351_REG4]);
    return(0);
}

//----------------------------------------------------------------------------
int Adf4351::SetMainPower( int pwr )
{
    mRegs[ADF4351_REG4] &= ~ADF4351_REG4_OUTPUT_PWR( 0x3 );
    mRegs[ADF4351_REG4] |=  ADF4351_REG4_OUTPUT_PWR( pwr&0x3 );
    (mSpiWriteFunc)(mDevNum,mLog&ADF_LOG_WRITES,mRegs[ADF4351_REG4]);
    return(0);
}

//----------------------------------------------------------------------------
int Adf4351::SetMainEnable( int enable )
{
    if( enable ){
        mRegs[ADF4351_REG4] |=  ADF4351_REG4_RF_OUT_EN;
    }
    else{
        mRegs[ADF4351_REG4] &= ~ADF4351_REG4_RF_OUT_EN;
    }
    (mSpiWriteFunc)(mDevNum,mLog&ADF_LOG_WRITES,mRegs[ADF4351_REG4]);
    return(0);
}

//----------------------------------------------------------------------------
int Adf4351::SetMtld( int enable )
{
    if( enable ){
        mRegs[ADF4351_REG4] |=  ADF4351_REG4_MUTE_TILL_LOCK_EN;
    }
    else{
        mRegs[ADF4351_REG4] &= ~ADF4351_REG4_MUTE_TILL_LOCK_EN;
    }

    (mSpiWriteFunc)(mDevNum,mLog&ADF_LOG_WRITES,mRegs[ADF4351_REG4]);
    return(0);
}

//----------------------------------------------------------------------------
int Adf4351::SetCpCurrent( int cur )
{
    mRegs[ADF4351_REG2] &=  ~(0xf<<9);
    mRegs[ADF4351_REG2] |=   ((cur&0xf)<<9);
    (mSpiWriteFunc)(mDevNum,mLog&ADF_LOG_WRITES,mRegs[ADF4351_REG2]);
    return(0);
}

//----------------------------------------------------------------------------
int Adf4351::SetPowerDown( int down )
{
    if( down ){
        mRegs[ADF4351_REG2] |=   ADF4351_REG2_POWER_DOWN_EN;
    }
    else{
        mRegs[ADF4351_REG2] &=  ~ADF4351_REG2_POWER_DOWN_EN;
    }
    (mSpiWriteFunc)(mDevNum,mLog&ADF_LOG_WRITES,mRegs[ADF4351_REG2]);
    return(0);
}

//----------------------------------------------------------------------------
/**
 * This method set the frequency to the specified value, waits up to usTimeout
 * microseconds for lock (in increments of us microseconds).
 *
 * The return value is the actual frequency set or 0 on error or timeout.
 */
int64_t Adf4351::SetFrequencyWithLock( int64_t freqHz, int dus, int usTimeout )
{
    int us;

    freqHz = SetFrequency( freqHz );

    us = 0;
    while( !GetLock() ){
       us_sleep(dus);
       us += dus;
       if( us>usTimeout ){
           return(0);
       }
    }

    return(freqHz);
}

//----------------------------------------------------------------------------
void Adf4351::Show( )
{
    ShowAdf4351Word(mDevNum, mRegs[ADF4351_REG0] );
    ShowAdf4351Word(mDevNum, mRegs[ADF4351_REG1] );
    ShowAdf4351Word(mDevNum, mRegs[ADF4351_REG2] );
    ShowAdf4351Word(mDevNum, mRegs[ADF4351_REG3] );
    ShowAdf4351Word(mDevNum, mRegs[ADF4351_REG4] );
    ShowAdf4351Word(mDevNum, mRegs[ADF4351_REG5] );
}

//----------------------------------------------------------------------------
int Adf4351::SetLowSpurMode( int lsmode )
{
    if( lsmode ){
        mRegs[ADF4351_REG2] |=   ADF4351_REG2_NOISE_MODE(0x3);
    }
    else{
        mRegs[ADF4351_REG2] &=  ~ADF4351_REG2_NOISE_MODE(0x3);
    }
    (mSpiWriteFunc)(mDevNum,mLog&ADF_LOG_WRITES,mRegs[ADF4351_REG2]);
    return(0);
}

