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
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include "SiCal.h"

//------------------------------------------------------------------------------
void SiCal::CalSetAdc( double dv )
{
    mAdCalFig = dv;
}

//------------------------------------------------------------------------------
void SiCal::CalSetIf( int gn, double dv )
{
   mIfCalFig[gn%RF_CAL_MAX_IFGAINS] = dv;
}

//------------------------------------------------------------------------------
void SiCal::CalSetCnv( int cn, double dv )
{
   mCnvCalFig[cn%RF_CAL_MAX_CNV] = dv;
}

//------------------------------------------------------------------------------
/**
 * This is the constructor for the calibration object.  Prior to use
 * it must be configured.  Unless otherwise updated it include default
 * reference calibration settings.
 */
SiCal::SiCal()
{
   int idx;

   // This is the conversion from dB16FS to dBm and
   // a function of the normalizations applied by the 
   // power spectral estimation technique
   //
   mAdCalFig    = 38.0;

   // Use the low gain reference unit measured values
   mIfCalFig[0] = -18.0;
   mIfCalFig[1] = -24.0;
   mIfCalFig[2] = -29.0;
   mIfCalFig[3] = -35.0;

   // Use the reference values for mixer+filter+amplifier
   mCnvCalFig[0] = +8.0;
   mCnvCalFig[1] =  0.0;
   mCnvCalFig[2] = -9.0;

   // Front end calibration
   mFeCalFig     = 0.0;

   // Use reference rf cal data
   CalSetVarReset();
   /*
   CalSetVarRef(    0.0,  -2.5 );
   CalSetVarRef(   36.0,  -2.5 );
   CalSetVarRef(   72.0,  -3.8 );
   CalSetVarRef(  108.0,  -3.8 );
   CalSetVarRef(  144.0,  -4.9 );
   CalSetVarRef(  180.0,  -3.9 );
   CalSetVarRef(  216.0,  -4.2 );
   CalSetVarRef(  252.0,  -4.4 );
   CalSetVarRef(  288.0,  -5.3 );
   CalSetVarRef(  324.0,  -6.1 );
   CalSetVarRef(  360.0,  -6.1 );
   CalSetVarRef(  396.0,  -6.7 );
   CalSetVarRef(  432.0,  -7.2 );
   CalSetVarRef(  469.0,  -8.7 );
   CalSetVarRef(  540.0, -10.1 );
   CalSetVarRef(  612.0,  -7.3 );
   CalSetVarRef(  684.0,  -7.1 );
   CalSetVarRef(  756.0,  -7.7 );
   CalSetVarRef(  828.0,  -7.8 );
   CalSetVarRef(  900.0,  -9.9 );
   CalSetVarRef( 1044.0,  -5.3 );
   CalSetVarRef( 1116.0,  -5.1 );
   CalSetVarRef( 1188.0,  -5.4 );
   CalSetVarRef( 1260.0,  -1.2 );
   CalSetVarRef( 1332.0,  +2.7 );
   CalSetVarRef( 1404.0,  +7.1 );
   CalSetVarRef( 1476.0,  +5.5 );
   CalSetVarRef( 1548.0,  +3.5 );
   CalSetVarRef( 1620.0,  +0.7 );
   CalSetVarRef( 1764.0,  +5.9 );
   CalSetVarRef( 1836.0,  +4.1 );
   CalSetVarRef( 4000.0,   0.0 );
   */
   CalSetVarRef(    0.0,  0.0 );
   CalSetVarRef( 4000.0,  0.0 );

   mCalRefStr = "dBm@Rfin";
}

//------------------------------------------------------------------------------
/**
 * This routine configures the frequency independent calibration
 * gain figure component based on the medium term configuration settings
 * It returns a string reflecting the reference in effect.
 */
const char *SiCal::CalConfigure( 
    int    nConversion, 
    int    ifGn,
    double feGain )
{
   int n;

   mCfgCalFig = mAdCalFig + mIfCalFig[ (ifGn&0x3) ] - feGain;

   for(n=0;n<nConversion;n++){
      mCfgCalFig += mCnvCalFig[n];
   }
   return(mCalRefStr);
}

//------------------------------------------------------------------------------
/**
 * This routine returns the calibration figure based on the configuration
 * and specified input frequency.  The value returned should be added
 * to a dBFS16 value to produe a dBm result.
 */
double SiCal::CalGet( double fHz )
{
   int idx;
   double c;

   idx = (int)(fHz+5e6)/1e7;
   if( idx<0 )                  idx=0;
   if( idx>=RF_CALBINS_10MHZ )  idx=(RF_CALBINS_10MHZ-1);
   c =  mCfgCalFig + m10MHzCalFig[idx];
// printf("c=%f, bin=%d, rf=%f f=%f\n",c,idx,m10MHzCalFig[idx],fHz);
   return(c);
}

//------------------------------------------------------------------------------
/**
 * This routine resets the rf reference calibration data.  After this
 * routine is invoked the monotonically increasing reference points
 * must be added.
 */
void SiCal::CalSetVarReset()
{
   int idx;

   // Clear the rf cal
   for(idx=0;idx<RF_CALBINS_10MHZ;idx++){
      m10MHzCalFig[ idx ] = 0.0;
   }
   mLastRefBin = 0;
}

//------------------------------------------------------------------------------
/**
 * This routine specifies a rf reference calibration point.  The calibration
 * data from the last input reference frequency to this point is linearly
 * captured.  For frequencies past the last input reference point the
 * default value of 0.0 is used.
 */
void SiCal::CalSetVarRef( double fMHz, int db )
{
   int bin,newRefBin;

   // Calculate and clamp new last reference bin
   newRefBin = (int)( (fMHz+5.0)/10.0 );
   if( newRefBin>=RF_CALBINS_10MHZ ) newRefBin = (RF_CALBINS_10MHZ-1);
   if( newRefBin<=0 ) return;

   // Save this reference point
   m10MHzCalFig[ newRefBin ] = db;

   // Work through the bins using linear interpolation
   for( bin=mLastRefBin; bin<newRefBin; bin++ ){
      m10MHzCalFig[ bin ] = 
                m10MHzCalFig[mLastRefBin]   +
                (
                  (bin - mLastRefBin)         * 
                  ( m10MHzCalFig[newRefBin] - m10MHzCalFig[mLastRefBin] )/
                  ( newRefBin - mLastRefBin )
                );
// printf("[%d %f]\n",bin,m10MHzCalFig[bin]);
   }

   // Save this as the last reference bin
   mLastRefBin = newRefBin;
}

int SiCal::CalSave( const char *fname )
{
   FILE *fp;
   int   idx;

   fp = fopen( fname, "w");
   if( !fp ) return( -1 );

   fprintf(fp,"# Calibration data\n");
   fprintf(fp,"si-cal-adc     %f\n", mAdCalFig );
   fprintf(fp,"\n");

   for( idx=0; idx<RF_CAL_MAX_IFGAINS; idx++ ){
      fprintf(fp,"si-cal-if  %d %f\n", idx, mIfCalFig[idx] );
   }
   fprintf(fp,"\n");

   for( idx=0; idx<RF_CAL_MAX_CNV; idx++ ){
      fprintf(fp,"si-cal-cnv %d %f\n", idx, mCnvCalFig[idx] );
   }
   fprintf(fp,"\n");

   fprintf(fp,"si-cal-var-reset\n");
   for( idx=0; idx<RF_CALBINS_10MHZ; idx++ ){
      fprintf(fp,"si-cal-var %f %f \n", idx*10.0, m10MHzCalFig[ idx ]);
   }

   fclose( fp );
   return( 0 );
}
