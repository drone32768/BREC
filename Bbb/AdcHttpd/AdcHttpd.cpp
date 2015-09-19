//
// This source code is available under the "Simplified BSD license".
//
// Copyright (c) 2015, J. Kleiner
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
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <microhttpd.h>

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"

#include "Devs.h"
#include "Pse.h"

////////////////////////////////////////////////////////////////////////////////
/// HwModel ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/**
 * This class represents the hardware model for the device.
 * It has its own thread for scanning (as derived from McF).
 */
class HwModel : public McF {

private:
#   define         HWM_LOG_SETOUTPUT 0x00000010
#   define         HWM_LOG_SWEEP     0x00000020
    unsigned int   mLog;   // Logging level for hw model

    int            mRun;         // on/off running with hw
    int            mNave ;       // Number of averages
    int            mChnl;        // Selected chnl
    int            mParamChange; // Flag indicating state has changed
    char          *mCfgFname;

    int            mXyCurLen;    // number of points in xy vectors
    int            mXyMaxLen;    // maximum points in xy vectors

    double         *mXvec;       // x values
    double         *mYvec;       // y values

    double         mXmin,mXmax;  // x limits determined by processing
    double         mYmin,mYmax;  // y limits determined by processing

    Pse            mPse;         // power spectrum estimator object
    double         mF1Hz;        // mixer 1 lo frequency

    // Internal support routines
    int            HwInit();
    void           HwStop();
    void           ShowState();
    int            ReadCfg();
    int            WriteCfg();

public:

    // Zero arg constructor
    HwModel();

    // Reqired interface elements for mcf
    void   Main();
    void   RcvEvent( char *evtStr );

    // Primary public interface
    int    SetState( char *name, char *value );
    int    GetState( char *resultsStr, int resultsLen );
    int    SetCfg( const char *fname );
};

/**
 * Constructor for hardware model.  It initializes all of the internal
 * values.  The object must be started after it is created.
 */
HwModel::HwModel()
{
    int  idx;

    mLog        = 0x0; // 0xffffffff;

    mRun        = 0;
    mNave       = 1;
    mChnl       = 0;
    mF1Hz       = 2500000;
    mParamChange= 0;
    mCfgFname   = strdup( "local.cfg" );

    mXyMaxLen   = 8192;
    mXyCurLen   = 256;
    mXvec       = (double*)malloc( mXyMaxLen*sizeof(double) );
    mYvec       = (double*)malloc( mXyMaxLen*sizeof(double) );

    mXmin       = -50e3;
    mXmax       =  50e3;
    mYmin       = -160;
    mYmax       = 0;
    for( idx=0; idx<mXyCurLen; idx++){
       mXvec[ idx ] = mXmin + ((double)idx/2048.0)*(mXmax-mXmin);
       mYvec[ idx ] = -40 + idx%10; 
    }
}

int
HwModel::SetCfg( const char *fname )
{
    free(mCfgFname);
    mCfgFname = strdup( fname ); 
    return( 0 );
}

int  HwModel::ReadCfg()
{
    printf("HwModel::ReadCfg:Reading configuration file\n");
    printf("HwModel::ReadCfg:Configuration file read complete\n");

    return( 0 );
}

int
HwModel::WriteCfg()
{
    printf("HwModel:WriteCfg:Enter\n");
    printf("HwModel:WriteCfg:Exit\n");
    return(0);
}

/**
 * This is a required interface of the McF class.  It is the main entry
 * point of the thread associated with the object.  A dedicated thread
 * will invoke this entry point when the object is started.
 */
void  HwModel::Main()
{
    int     chnl;
    double  f1Hz;
 
    // Read and restore saved configuration
    ReadCfg();

    // Initialize the hw
    HwInit();

    // Main processing loop
    chnl = -1;
    f1Hz = -1.0;
    while( !mThreadExit ){

       if( mRun ){

          // Revisit any changed hw parameters
          if( mChnl!=chnl ){
             Dp()->Adc()->SetSource( mChnl );
             chnl = mChnl;
          }
          if( mF1Hz!=f1Hz ){
             Dp()->Mx1()->SetLoFreqHz( mF1Hz );
             f1Hz = mF1Hz;
          }

          // Conduct the specified processing
          mPse.ProcessCoherentInterval( 
                    mNave,
                    mXyCurLen,
                    mXvec,   
                    mYvec
          ); 
       } // End of running

       else{
          sleep( 1 );
       } // End of not running

    } // End of main processing loop 
}

/**
 * This is a required interface of the McF class. No McF events are processed
 * in this application.
 */
void HwModel::RcvEvent( char *evtStr )
{
   // No event processing
}

/**
 * This is the primary external interface to the model.  It is given
 * a single name/value pair (text strings). The model state is updated
 * based on the provided values.
 */
int
HwModel::SetState( char *name, char *value )
{
    int   err;

    printf("HwModel::SetState name=\'%s\',value=\'%s\'\n",name,value);

    // TODO - there really should be a lock around this method as well as
    //        the param change indicator elsewhere

    if( 0==strcmp(name,"run") ){
          if( 0==strcmp(value,"ON") ){
             mRun = 1;
          }
          else{
             mRun = 0; 
          }
    }

    else if( 0==strcmp(name,"nPts") ){
          int nPts;
          nPts = mXyCurLen;
          nPts = nPts * 2;
          if( nPts>4096 ) nPts = 256;
          mXyCurLen = nPts;
    }

    else if( 0==strcmp(name,"nAve") ){
          int nAve;
          nAve = mNave;
          nAve = nAve * 2;
          if( nAve>8 ) nAve = 1;
          mNave = nAve;
    }

    else if( 0==strcmp(name,"chnl") ){
          mChnl = (mChnl+1)%8;
    }

    else if( 0==strcmp(name,"f1Hz") ){
          mF1Hz = atof( value );
    }

    else if( 0==strcmp(name,"swreset") && 0==strcmp(value,"ON") ){
          exit(0);
    }

    else if( 0==strcmp(name,"savecfg") && 0==strcmp(value,"ON") ){

	  err = system("/bin/mount -o remount,rw /");
          if( -1 == err ){
              fprintf(stderr,"system command error [mount rw]\n");
          }

          WriteCfg();

	  err = system("/bin/mount -o remount,ro /");
          if( -1 == err ){
              fprintf(stderr,"system command error [mount ro]\n");
          }
    }

    else if( 0==strcmp(name,"hwreset") && 0==strcmp(value,"ON") ){
	  err = system("/sbin/shutdown -r now");    
          if( -1 == err ){
              fprintf(stderr,"system command error [shutdown -r]\n");
          }
    }

    else if( 0==strcmp(name,"shutdown") && 0==strcmp(value,"ON") ){
	  err = system("/sbin/shutdown -h now");    
          if( -1 == err ){
              fprintf(stderr,"system command error [shutdown -h]\n");
          }
    }

    mParamChange = 1;
    return( 0 );
}

/**
 * This is the primary external interface to the model.  It places
 * the model state in a provided string.  The state is formated
 * as a JSON object.
 */
int
HwModel::GetState( char *resultsStr, int resultsLen )
{
    int    nBytes;
    char  *pos;
    int    len;
    int    idx;
    time_t now;
    char   timeStr[128];

    time( &now );
    ctime_r( &now, timeStr );
    timeStr[ strlen(timeStr) - 1 ] = 0; // remove new line

    int npc;
    static int npi = 0; // FIXME

#   define MAX_NPTS_PER_GET 512
    npc = MAX_NPTS_PER_GET;
    npi = (npi+npc)%mXyCurLen;

    // Setup the output position and residual length
    pos = resultsStr;
    len = resultsLen;

    // Output the 
    nBytes = snprintf(pos, len, 
                "{ "
                    "\"savecfg\"  : \"OFF\","
                    "\"swreset\"  : \"OFF\","
                    "\"hwreset\"  : \"OFF\","
                    "\"shutdown\" : \"OFF\","
                    "\"run\"      : \"%s\","
                    "\"time\"     : \"%s\","
                    "\"nAve\"     : \"%d\","
                    "\"chnl\"     : \"%d\","
                    "\"nPts\"     : \"%d\","
                    "\"f1Hz\"     : \"%f\","
                    "\"tpc\"      : %d,"      
                    "\"npc\"      : %d,"      
                    "\"npi\"      : %d,"    ,
                    mRun?"ON":"OFF",            // run
                    timeStr,                    // time
                    mNave,                      // nAve
                    mChnl,                      // chnl
                    mXyCurLen,                  // nPts
                    mF1Hz,                      // f1Hz
                    mXyCurLen,                  // total point count
                    npc,                        // new point count
                    npi                         // new point index
    );
    pos += nBytes;
    len -= nBytes;

    // TODO - these should not be necessary
    mXmin = mXvec[0];
    mXmax = mXvec[mXyCurLen-1]; // FIXME PSE should be able to set
    nBytes = snprintf(pos, len, "\"lim\":[%g,%g,%g,%g],",
                                    mXmin,mXmax,mYmin,mYmax);
    pos += nBytes;
    len -= nBytes;

    nBytes = snprintf(pos, len, "\"npx\":[");
    pos += nBytes;
    len -= nBytes;

    for(idx=0;idx<npc;idx++){
        nBytes = snprintf(pos, len, "%g%c",mXvec[npi+idx],
                                           (idx==(npc-1))?' ':',');
        pos += nBytes;
        len -= nBytes;
    }

    nBytes = snprintf(pos, len, "],"); // End of npx
    pos += nBytes;
    len -= nBytes;

    nBytes = snprintf(pos, len, "\"npy\":[");
    pos += nBytes;
    len -= nBytes;

    for(idx=0;idx<npc;idx++){
        nBytes = snprintf(pos, len, "%g%c",mYvec[npi+idx],
                                           (idx==(npc-1))?' ':',');
        pos += nBytes;
        len -= nBytes;
    }

    nBytes = snprintf(pos, len, "]"); // End of npy
    pos += nBytes;
    len -= nBytes;

    nBytes = snprintf(pos, len, "}"); // End of main obj
    pos += nBytes;
    len -= nBytes;

    return(0);
}

/**
 * This internal method dumps the internal model state to stdout
 */
void HwModel::ShowState()
{
    printf("HwModel::ShowState:mXyCurLen= %d\n",mXyCurLen);
    printf("HwModel::ShowState:mNave    = %d\n",mNave );
}

/**
 * This internal method initializes the hardware. It must be invoked
 * prior to conducting any operations on the model.
 */
int HwModel::HwInit()
{
    Dp()->Open();

    mChnl = Dp()->Adc()->SetSource( 0 );

    mF1Hz = Dp()->Mx1()->SetLoFreqHz( 1.0e6 );

    // TODO - complex and saving actual rate
    Dp()->Adc()->SetComplexSampleRate( 5000000 );

    return(0);
}

/**
 * This is the internal method used to place the hardware in 
 * a quiescent state.
 */
void HwModel::HwStop()
{
}


////////////////////////////////////////////////////////////////////////////////
/// HTTP Interface /////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define         LOG_MASK_ACCESS  0x10000000
#define         LOG_MASK_FILE    0x08000000
unsigned int    gLogMask = 0;
HwModel        *gHwm;

static char gErrPage[]=
"<html>"
"<head><title>File not found</title></head>"
"<body>File not found</body>"
"</html>";

static ssize_t
file_reader (void *cls, uint64_t pos, char *buf, size_t max)
{
  FILE *file = (FILE*)cls;

  if( gLogMask&LOG_MASK_FILE ){
      printf("file_reader:\n");
  }

  (void)  fseek (file, pos, SEEK_SET);
  return fread (buf, 1, max, file);
}

static void
free_callback (void *cls)
{
  if( gLogMask&LOG_MASK_FILE ){
      printf("file_callback:\n");
  }

  FILE *file = (FILE*)cls;
  fclose (file);
}

static int
ahc_access_handler (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data,
	  size_t *upload_data_size, void **ptr)
{
   struct MHD_Response *response;
   int                  ret;
   const char          *fname;
   FILE                *file;
   struct stat          buf;
#  define MAX_RESP_BYTES 65536
   char                 rstr[MAX_RESP_BYTES];
 
   if( gLogMask&LOG_MASK_ACCESS ){
      printf("ahc_access_handler:%s <%s>\n", method, &url[1]);
   }

   if (0 != strcmp (method, MHD_HTTP_METHOD_GET)){
     return MHD_NO;             
   }

   /*  TODO re-evaluate impacts of this
   static int           aptr;
   if (&aptr != *ptr) {
       *ptr = &aptr;
       return MHD_YES;
   }
   */

   /* reset when done */
   *ptr = NULL;                  


   //////////////////////////////////////// 
   if( 0==strcmp("setstate", &url[1]) ){
      char       *inStr;
      char       *name,*value;

      const char *vstr;
      char       *end;

      vstr = MHD_lookup_connection_value( 
                        connection, 
                        MHD_GET_ARGUMENT_KIND,
                        "v"
                      );
      // printf("ahc_access_handler:setstate v=\'%s\'\n",vstr);

      inStr = strdup( vstr );
      
      // NOTE: this doesn't parse a true JSON object
      // URL is expected to be setstate?v=name:value
      name  = strtok_r( inStr, ":", &end );
      value = strtok_r( NULL, "",   &end );

      // printf("ahc_access_handler: name=\'%s\',value=\'%s\'\n",name,value);

      gHwm->SetState( name, value ); 

      sprintf(rstr,"ok");
      response = MHD_create_response_from_buffer (strlen (rstr),
 						  (void *) rstr,
 						  MHD_RESPMEM_MUST_COPY);
      ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
      MHD_destroy_response (response);

      free(inStr);
      return ret;
   }

   //////////////////////////////////////// 
   else if( 0==strcmp("getstate", &url[1]) ){

      // printf("ahc_access_handler:getstate\n");

      gHwm->GetState(rstr, sizeof(rstr) - 1 );
      response = MHD_create_response_from_buffer (strlen (rstr),
 						  (void *) rstr,
 						  MHD_RESPMEM_MUST_COPY);
      ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
      MHD_destroy_response (response);
      return ret;
   }

   //////////////////////////////////////// 
   else if( 0==url[1] ){
      fname = "index.html";
   }

   //////////////////////////////////////// 
   else {
      // TODO: this should limit the directory extent of file name
      fname = &url[1];
   }

   if (0 == stat (fname, &buf)){
     file = fopen (fname, "rb");
   }
   else {
     file = NULL;
   }

   if (file == NULL) {
       response = MHD_create_response_from_buffer (strlen (gErrPage),
 						  (void *) gErrPage,
 						  MHD_RESPMEM_PERSISTENT);
       ret = MHD_queue_response (connection, MHD_HTTP_NOT_FOUND, response);
       MHD_destroy_response (response);
   }
   else {
       response = MHD_create_response_from_callback (
                             buf.st_size, 32 * 1024,     /* 32k page size */
                             &file_reader,
                             file,
                             &free_callback);
       if (response == NULL) {
 	  fclose (file);
 	  return MHD_NO;
       }
       ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
       MHD_destroy_response (response);
   }

   return ret;
}

void usage( int exit_code )
{
   fprintf(stderr,
"This utility implement a simple http based adc utility\n"
"Point a web browser at the platform address and specified port\n"
"-port <port>     http port to use (8080 default)\n"
"-config <fname>  configurtion file name\n"
);
   exit(0);
}

int
main (int argc, char **argv)
{
   struct MHD_Daemon *d;
   int                idx;
   int                port;
   const char        *cfgFname;
 
   // Default arguments
   port     = 8080;
   cfgFname = "syn.cfg";

   // Parse arguments
   idx = 1;
   while( idx< argc ){
       if( 0==strcmp(argv[idx],"-h") ){ 
          usage(0); 
          idx++;
          continue;
       }
       else if( 0==strcmp(argv[idx],"-help") ){ 
          usage(0); 
          idx++;
          continue;
       }
       else if( 0==strcmp(argv[idx],"-port") ){ 
          if((idx+1)>=argc ) usage(-1);
          port = atoi(argv[idx+1]);
          idx+=2;
          continue;
       }
       else if( 0==strcmp(argv[idx],"-config") ){ 
          if((idx+1)>=argc ) usage(-1);
          cfgFname = argv[idx+1];
          idx+=2;
          continue;
       }
       else{
          fprintf(stderr,"unrecogized arg %s\n",argv[idx]);
          idx++;
       }
   }

   // Show arguments
   printf("port   = %d\n",port);
   printf("config = %s\n",cfgFname);

   // Create and start the hardware model
   gHwm = new HwModel();
   gHwm->SetCfg( cfgFname );
   gHwm->Start();

   // Create and start the http server
   d = MHD_start_daemon (
                // MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG,
                MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG,
                port,
                NULL,      NULL, 
                &ahc_access_handler, 
                (void*)gErrPage, 
                MHD_OPTION_END
       );

   if (d == NULL){
      return(-1);
   }

   while(1){ sleep(60); }
   return(0);
}
