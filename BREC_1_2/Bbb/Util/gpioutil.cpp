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
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "gpioutil.h"

GpioUtil::GpioUtil()
{
    mFd    = -1;
    mGpioN = 0;
}

void GpioUtil::Define( int gpion )
{
    mGpioN = gpion;
}

int GpioUtil::Export()
{
    char    tmpStr[128];
    int     fd;
    int     nb;

#   ifdef TGT_ARM
       sprintf( tmpStr, "/sys/class/gpio/export" );
#   else
       sprintf( tmpStr, "/dev/null" );
#   endif

    fd = open( tmpStr, O_WRONLY ); 
    if( fd < 0 ){
        fprintf(stderr,"GpioUtil:Open:error opening export on gpio %d\n",
                      mGpioN);
        perror("GpioUtil:Open:open(export)");
        return( -1 );
    }
    else{
        sprintf( tmpStr, "%d", mGpioN );
        nb = write( fd, tmpStr, strlen(tmpStr) ); 
        /*
         * 140605 - previously exported pins appear busy to a new export
        */
        if( nb!=(int)strlen(tmpStr) && errno!=EBUSY ){
            fprintf(stderr,"GpioUtil:Open:error exporting gpio %d\n",mGpioN);
            perror("GpioUtil:Open:write");
        }
        close( fd );
    }
    return( 0 );
}

int GpioUtil::SetDirInput( int isInput )
{
    char    tmpStr[128];
    int     fd;
    int     nb;

    /////////////////////////////////////////
    /// Direction
    /////////////////////////////////////////
#   ifdef TGT_ARM
       sprintf( tmpStr, "/sys/class/gpio/gpio%d/direction", mGpioN );
#   else
       sprintf( tmpStr, "/dev/null" );
#   endif
    fd = open( tmpStr, O_WRONLY );
    if( fd < 0 ){
        fprintf(stderr,"GpioUtil:Open:error opening direction on gpio %d\n",
                      mGpioN);
        perror("GpioUtil:Open:open(direction)");
        return(-1);
    }
    else{
        if(isInput){
            sprintf( tmpStr, "in" );
        }
        else{
            sprintf( tmpStr, "out" );
        }
        nb = write( fd, tmpStr, strlen(tmpStr) ); 
        if( nb!=(int)strlen(tmpStr)  ){
            fprintf(stderr,"GpioUtil:Open:error direction gpio %d\n",mGpioN);
            perror("GpioUtil:Open:write");
        }
        close( fd );
    }
    return( 0 );
}

int GpioUtil::Open()
{
    char    tmpStr[128];

    /////////////////////////////////////////
    /// Pin
    /////////////////////////////////////////
#   ifdef TGT_ARM
       sprintf( tmpStr, "/sys/class/gpio/gpio%d/value", mGpioN );
#   else
       sprintf( tmpStr, "/dev/null" );
#   endif
    mFd = open( tmpStr, O_RDWR ); 
    if( mFd < 0 ){
        fprintf(stderr,"GpioUtil:Open:error opening gpio %d\n",mGpioN);
        perror("GpioUtil:Open:open(gpio pin)");
    }

    return( 0 );
}

int GpioUtil::Open( int gpion )
{
    mGpioN = gpion;
    return( Open() );
}

int GpioUtil::Close()
{
    if( mFd > 0 ) close( mFd );
    mFd = -1;
    return( 0 );
}

int GpioUtil::Set( int v )
{
    int nb;

    nb = write( mFd, v?"1":"0", 1 );
    if( 1!=nb ) {
        fprintf(stderr,"GpioUtil:Set error writing gpio %d\n", mGpioN);
        perror("GpioUtil:Set:write");
        return( -1 );
    }
    return( 0 );
}

int GpioUtil::Get()
{
    int  nb;
    char tmpStr[4];

    tmpStr[0]='x';

    lseek( mFd, 0, SEEK_SET );
    nb=read( mFd, tmpStr, 1 );
    if( nb<0 ){
        fprintf(stderr,"GpioUtil:Set error reading gpio %d\n", mGpioN);
        perror("GpioUtil:Set:read");
        return( -1 );
    }
    if( '0'==tmpStr[0] ) return(0);
    if( '1'==tmpStr[0] ) return(1);
    return(-1);
}

