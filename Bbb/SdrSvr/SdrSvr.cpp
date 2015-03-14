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
///
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "AscpCtlIf.h"
#include "AscpDatIf.h"
#include "AscpDisIf.h"
#include "CliCtlIf.h"
#include "Device.h"

////////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] )
{
    AscpCtlIf *ascpCtl;
    AscpDatIf *ascpDat;
    AscpDisIf *ascpDis;
    CliCtlIf  *cif;
    Device    *dev;
    EvR       *evr;

    evr = GetEvrSingleton();

    // Create each of the core objects/interfaces
    cif     = new CliCtlIf();
    dev     = new Device();
    ascpCtl = new AscpCtlIf();
    ascpDat = new AscpDatIf();
    ascpDis = new AscpDisIf();

    // Set the service ports for those that need it
    ascpCtl->SetSvcPort( 6788 );
    cif->SetSvcPort(     6789 );

    // Associate the device with the data interface
    ascpDat->SetDevice( dev );

    // Register all the objects with the event
    // NOTE: lower level objects may self register of necessity
    evr->Register( cif );
    evr->Register( dev );
    evr->Register( ascpCtl );
    evr->Register( ascpDat );
    evr->Register( ascpDis );

    // Start all of the object threads
    cif->Start();
    dev->Start();
    ascpCtl->Start();
    ascpDat->Start();
    ascpDis->Start();

    // Set the name via event
    char lineBf[128];
    snprintf(lineBf,sizeof(lineBf)-1,"OEM-%d",getpid() );
    ascpCtl->SetRadioName( lineBf );

    while( 1 ){ sleep(10); }
}
