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
#include <pthread.h>
#include <math.h>

#include "../Util/mcf.h" // for us_sleep
#include "RemoteDev.h"

//------------------------------------------------------------------------------
/**
 * This method creates a remote device object.  The remote address is
 * defaulted to 192.168.0.3:6790
 */
RemoteDev::RemoteDev()
{
   mTcc          = NULL;
   snprintf( mIpAddrStr, sizeof(mIpAddrStr), "192.168.0.3" );
   mIpAddrPort = 6790;
   mLastConnectSecs = 0;
}

//------------------------------------------------------------------------------
/**
 * This method sets the remote device address to the values specified.
 * If the device is already connected it will be disconnected.
 *
 */
void RemoteDev::SetAddr( const char *ipAddrStr, int port )
{
   if( mTcc ) Disconnect();

   strncpy(mIpAddrStr, ipAddrStr, sizeof(mIpAddrStr) - 1 );
   mIpAddrPort = port;
}

//------------------------------------------------------------------------------
/**
 * This method returns a pointer to the ip address string of the device
 */
char * RemoteDev::GetAddr()
{
   return(mIpAddrStr);
}

//------------------------------------------------------------------------------
/** 
 * This method returns the current error status of the remote device
 *  0 = ok
 * -1 = unspecified error/not connected
 */
int RemoteDev::Error()
{
   if( mTcc ) return(0);
   return( -1 );
}

//------------------------------------------------------------------------------
/**
 * This method connects to the currently configured remote address
 * Returns: 0 on success, <0 otherwise
 */
int RemoteDev::Connect()
{
   int err, nb, cmdStrBytes;

   // Create a new connection record
   mTcc = TcpClnConNew();

   // Try to connect with timeout
   err = TcpClnConnect( mTcc, mIpAddrStr, mIpAddrPort, 100 );

   // Cleanup if error
   if( err ){
      TcpClnConDelete( mTcc );
      mTcc = NULL;
      return(-1);
   }

   // Create  the command for mute until lock detect
   cmdStrBytes = snprintf( 
                           (char*)mCmdStr,
                           sizeof(mCmdStr),
                           "wb-syn-mtld 1 1\n"
                         );

   // Send the command
   nb = TcpClnSend( mTcc, (unsigned char*)mCmdStr, cmdStrBytes );

   // If this didn't fully work, clear the connect to be retried later
   // and abort here.
   if( nb!=cmdStrBytes ){
      TcpClnConDelete( mTcc );
      mTcc = NULL;
      return( -1 );
   }

   // Read and discard any thing coming back
   err=ReadLine(mCmdStr,sizeof(mCmdStr),100);
   printf("RemoteDev:Connect:%s err=%d str=%s",mIpAddrStr,err,mCmdStr);
   
   // Connect was successfull
   return( 0 );
}

//------------------------------------------------------------------------------
int RemoteDev::ReadLine( char *lineBf, int lineBytes, int timeOutMs )
{
   char *rBf;
   int   rBytes;
   int   nb;
   int   ms;

   rBf    = lineBf;
   rBytes = 0;
   ms     = 0;
   while( 1 ){

      // Abort if we have waited more than timeout
      if( ms > timeOutMs ){ 
         return(-2);
      }

      // Try reading a character
      nb = TcpClnRecv( mTcc, (unsigned char*)rBf, 1 );

      // Depending on what we got process
      switch( nb ){

         // Rcv character
         case 1:{
            if( '\n' == *rBf ){
               rBf[1]=0;
               return(0);
            }
            rBytes++;
            if( rBytes < (lineBytes-1) ){
               rBf++;
            }
         }
         break;

         // Nothing to read
         case 0:{
            // nothing read, just sleep
            us_sleep( 10000 );
            ms+=10;
         }
         break;

         // Error on read
         default:{
            return(-1);
         }
      }

   }
   return(-1);
}

//------------------------------------------------------------------------------
/**
 * This method connects from a currently connected remote device
 * Returns 0 on success, <0 otherwise
 */
int RemoteDev::Disconnect()
{
   if( !mTcc ) return(0);

   TcpClnConDelete( mTcc );
   mTcc = NULL;
   return(0);
}

//------------------------------------------------------------------------------
double RemoteDev::SetFreqHz( int ln, double fHz )
{
   time_t        nowS;
   int           err, nb, cmdStrBytes;
   double        fHzAct;

   // Try connect if not already connected
   // Don't try this more often than once a second
   if( NULL==mTcc ){
       nowS = time(NULL); 
       if( (nowS - mLastConnectSecs) > 1 ){
          mLastConnectSecs = nowS;
          err = Connect();
          if( err ){
              printf("RemoteDev:SetFreqHz:%s connect fail\n",mIpAddrStr);
              return( err );
          }
       }
       else{
          return( -1.0 );
       }
   }

   // Create  the command for given frequency
   cmdStrBytes = snprintf( 
                           (char*)mCmdStr,
                           sizeof(mCmdStr),
                           "wb-syn-freq-hz %d %u\n", 
                           ln,
                           (unsigned int)fHz 
                         );

   // Send the command
   nb = TcpClnSend( mTcc, (unsigned char*)mCmdStr, cmdStrBytes );

   // If this didn't fully work, clear the connect to be retried later
   // and abort here.
   if( nb!=cmdStrBytes ){
      TcpClnConDelete( mTcc );
      mTcc = NULL;
      printf("RemoteDev:SetFreqHz:%s send error\n",mIpAddrStr);
      return( -2.0 );
   }

   // Get the response
   err = ReadLine( mCmdStr, sizeof(mCmdStr), 100 );

   printf("RemoteDev:SetFreqHz:%s err=%d, ret=%s",
                mIpAddrStr,err,mCmdStr);

   // Convert the results 
   fHzAct = atof( mCmdStr );
   
   // Successfully set
   return(fHzAct);
}

