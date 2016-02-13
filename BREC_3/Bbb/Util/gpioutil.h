/*
 *
 * This source code is available under the "Simplified BSD license".
 *
 * Copyright (c) 2013, J. Kleiner
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the original author nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __GPIO_UTIL__
#define __GPIO_UTIL__

#include "Interfaces/GpioPin.h"

/**
 * GpioUtil - simple class to manage the sysfs gpio programming
 */
class GpioUtil : public GpioPin {

  private:
    int mGpioN;
    int mFd;

  public:

    /** Constructor.  This does no IO, and accesses no devices of files 
     */
    GpioUtil();

    /** This method defines the number of the gpio.  It is the number
     *  used in the sysfs file path to the pin
     */
    void Define( int gpion );

    /** This method exports the pin in sysfs
     */
    int Export();

    /** This method sets the direction of the pin
     */
    int SetDirInput( int isInput );

    /** This method closes the sysfs file 
     */
    int Close();

    /** This method opens the pin 
     */
    int Open();

    /** This method defines and opens the pin.  The export and direction
     *  must have been set elsewhere (it is intended for applications
     *  where the device tree has already set the direction and it is fixed
     */
    int Open( int gpion );

    /** Set the pin value 
     */
    int Set( int v );

    /** Get the pin value 
     */
    int Get( );
};

/**
 * Gpio6PinGroup - class to represent a set of 6 gpio's for board level intf
 */
class Gpio6PinGroup 
{
   private:
   int      mGpioGroupId;

   // Outputs
   GpioUtil mGpioMoSi;
   GpioUtil mGpioSclk;
   GpioUtil mGpioSs1;
   GpioUtil mGpioSs2;

   // Inputs
   GpioUtil mGpioMiSo;
   GpioUtil mGpioStat;

   // Direction to be defined

   public:
   int       GetGroupId() { return( mGpioGroupId ); }
   GpioUtil *GetMoSi()  { return(&mGpioMoSi); }
   GpioUtil *GetSclk()  { return(&mGpioSclk); }
   GpioUtil *GetSs1()   { return(&mGpioSs1);  }
   GpioUtil *GetSs2()   { return(&mGpioSs2);  }

   GpioUtil *GetMiSo()  { return(&mGpioMiSo); }
   GpioUtil *GetStat()  { return(&mGpioStat); }
 
   void       DefineGroupId( int n ){ mGpioGroupId = n;     }

   void       DefineMoSi(   int n ){ mGpioMoSi.Define(n);   }
   void       DefineSclk( int n )  { mGpioSclk.Define(n);   }
   void       DefineSs1(    int n ){ mGpioSs1.Define(n);    }
   void       DefineSs2(  int n )  { mGpioSs2.Define(n);    }
   void       DefineMiSo(   int n ){ mGpioMiSo.Define(n);   }
   void       DefineStat(  int n ) { mGpioStat.Define(n);   }
};

#endif /* __GPIO_UTIL__ */

