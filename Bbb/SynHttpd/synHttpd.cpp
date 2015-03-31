
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
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <microhttpd.h>

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"

#include "../Iboard/Iboard.h"
#include "../Lboard/Lboard.h"
#include "../Mboard/Mboard.h"
#include "../Adf4351/Adf4351.h"

////////////////////////////////////////////////////////////////////////////////
/// HwModel ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/**
 * This class represents the hardware model for the level controlled 
 * synthesizer.  It has its own thread for scanning (as derived from McF).
 * It uses an I board with M board and L board (with power meter) at fixed 
 * ports.
 */
class HwModel : public McF {

private:
    int            mLport; // I board port of L board
    int            mMport; // I board port of M board

    Gpio6PinGroup *mg6pg;  // Pin group for discrete io on I board
    Iboard        *mIbrd;  // I board for interfacing to others
    Lboard        *mLbrd;  // L board for level control
    Mboard        *mMbrd;  // M board for synthesizer output
    Adf4351       *mSyn;   // synthisizer object on M board
    int            mNest;  // Number of estimates for power reading
#   define         HWDEVS_LOG_SETOUTPUT 0x00000010
#   define         HWDEVS_LOG_SWEEP     0x00000020
    unsigned int   mLog;   // Logging level for hw model

    int            mRun;       // on/off running with hw
    double         mActHz;     // Actual frequency in Hz
    double         mActDbm;    // Actual output level in dBm
    int            mMonitor;   // Flag for continuous monitor in effect
    int            mSweep;     // Flag for if sweeping in effect
    int            mLockStatus; // Last lock status obtained from syn
    double         mTargetHz;  // Intended frequency of last sweep step
    double         mStartHz;   // Frequency to start sweep
    double         mTargetDbm; // Intended level of last sweep step
    double         mStartDbm;  // Level to start sweep
    double         mStopHz;    // Frequency to stop sweep at
    double         mStopDbm;   // Level to stop sweep at
    double         mStepHz;    // Frequency increment in sweep
    double         mStepDbm;   // Level increment in sweep
    int            mDwellMs;   // Time in mS for a given sweep step
    int            mParamChange; // Flag indicating state has changed
    char          *mCfgFname;

    int    ReadCfg();
    int    WriteCfg();

public:

    // Reqired interface elements for mcf
    HwModel();
    void   Main();
    void   RcvEvent( char *evtStr );

    // Primary public interface
    int    SetState( char *name, char *value );
    int    GetState( char *resultsStr, int resultsLen );
    int    SetCfg( const char *fname );

    // Internal support routines
    int    HwInit();
    void   HwStop();
    void   ShowState();
    void   DoSweepStep();
    void   DoSweepDwellTime();
    double GetPwrEst( int nEst );
    int    SetOutput( double targetHz, double targetDbm );

};

/**
 * Constructor for hardware model.  It initializes all of the internal
 * values.  The object must be started after it is created.
 */
HwModel::HwModel()
{
    mLport       = 0;
    mMport       = 2;

    mLog        = 0x20; // 0xffffffff;
    mg6pg       = NULL;
    mIbrd       = NULL;
    mLbrd       = NULL;
    mMbrd       = NULL;
    mSyn        = NULL;
    mNest       = 10;

    mRun        = 0;
    mActHz      = 100;
    mActDbm     = -0.1;
    mMonitor    = 0;
    mSweep      = 0;
    mStartHz    = 100000000;
    mTargetHz   = mStartHz;
    mStartDbm   = -1;
    mTargetDbm  = mStartDbm;
    mStopHz     = mStartHz  + 10000000;
    mStopDbm    = mStartDbm - 10;
    mStepHz     = 300000;
    mStepDbm    = -0.25;
    mDwellMs    = 100;
    mParamChange= 0;
    mCfgFname   = strdup( "x.cfg" );
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
    FILE *fp;
    char  *name, *value,*end;
    char  lineBf[1024];

    // printf("Reading configuration file\n");
    fp = fopen( mCfgFname, "r" );
    if( !fp ){
        fprintf(stderr,"cannot open configuration file\n");
        return(-1);
    }

    while( fgets(lineBf,sizeof(lineBf)-1,fp) ){
        name  = strtok_r( lineBf, " ", &end );
        value = strtok_r( NULL, " \n",   &end );
        SetState( name, value ); 
    }

    fclose(fp);
    // printf("Configuration file read complete\n");

    return( 0 );
}

int
HwModel::WriteCfg()
{
    FILE *fp;

    printf("WriteCfg:Enter\n");

    fp = fopen( mCfgFname, "w" );
    if( !fp ){
        fprintf(stderr,"cannot open %s\n",mCfgFname);
        return( -1 );
    }

    fprintf(fp, 
                    "run        %s\n"
                    "actHz      %.0f\n"
                    "actDbm     %.3f\n"
                    "sweep      %s\n"
                    "monitor    %s\n"
                    "startHz    %.0f\n"
                    "startDbm   %.3f\n"
                    "stopHz     %.0f\n"
                    "stopDbm    %.3f\n"
                    "stepHz     %.0f\n"
                    "stepDbm    %.3f\n"
                    "dwellMs    %d\n",
                    mRun?"ON":"OFF",            // run
                    mActHz,                     // actHz
                    mActDbm,                    // actDbm
                    mSweep?  "ON":"OFF",        // sweep
                    mMonitor?"ON":"OFF",        // monitor
                    mStartHz,                   // startHz
                    mStartDbm,                  // startDbm
                    mStopHz,                    // stopHz
                    mStopDbm,                   // stopDbm
                    mStepHz,                    // stepHz
                    mStepDbm,                   // stepDbm
                    mDwellMs                    // dwellMs
    );

    fclose(fp);
    printf("WriteCfg:Exit\n");
    return(0);
}

/**
 * This is a required interface of the McF class.  It is the main entry
 * point of the thread associated with the object.  A dedicated thread
 * will invoke this entry point when the object is started.
 */
void  HwModel::Main()
{

    // Read and restore saved configuration
    ReadCfg();
    if( mRun ){
        HwInit();
        SetOutput( mStartHz, mStartDbm );
    }

    // Main processing loop
    while( !mThreadExit ){

        // ShowState();

        // If not running just wait here
        if( !mRun ){
           while( !mRun && !mThreadExit ){
               us_sleep( 50000 );
           }
           HwInit();
           SetOutput( mStartHz, mStartDbm );
        }

        if( mSweep ){
            DoSweepStep();
            DoSweepDwellTime();
        }
        else{
            if( mParamChange ){
               SetOutput( mStartHz, mStartDbm );
               mParamChange= 0;
            }

            // Always update lock since its a discrete
            mLockStatus = mSyn->GetLock();

            // Update level only if indicated
            if( mMonitor ){  
                mActDbm  = GetPwrEst(mNest);
            }

            us_sleep( 50000 );
        }

        // at this point we are guarenteed to have been running.
        // if we are now not supposed to be running, stop the
        // hardware and continue to the top of the processing loop
        // where we re-check run state and wait or init appropriately
        if( !mRun ){
           HwStop();
        }

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

    printf("SetState name=\'%s\',value=\'%s\'\n",name,value);

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

    else if( 0==strcmp(name,"monitor") ){
          if( 0==strcmp(value,"ON") ){
             mMonitor = 1;
          }
          else{
             mMonitor = 0; 
          }
    }

    else if( 0==strcmp(name,"sweep") ){
          if( 0==strcmp(value,"ON") ){
             mSweep = 1;
          }
          else{
             mSweep = 0;
          }
    }

    else if( 0==strcmp(name,"startHz") ){
          mStartHz =  atof(value);
    }

    else if( 0==strcmp(name,"startDbm") ){
          mStartDbm=atof(value);
    }

    else if( 0==strcmp(name,"stopHz") ){
          mStopHz=atof(value);
    }

    else if( 0==strcmp(name,"stopDbm") ){
          mStopDbm=atof(value);
    }

    else if( 0==strcmp(name,"stepHz") ){
          mStepHz = atof(value);
    }

    else if( 0==strcmp(name,"stepDbm") ){
          mStepDbm=atof(value);
    }

    else if( 0==strcmp(name,"dwellMs") ){
          mDwellMs=atoi(value);
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
    time_t now;
    char   timeStr[128];

    time( &now );
    ctime_r( &now, timeStr );
    timeStr[ strlen(timeStr) - 1 ] = 0; // remove new line

    snprintf(resultsStr, resultsLen, "{ "
                    "\"run\"      : \"%s\","
                    "\"lock\"     : \"%s\","
                    "\"time\"     : \"%s\","
                    "\"actHz\"    : %.0f,"
                    "\"actDbm\"   : %.3f,"
                    "\"sweep\"    : \"%s\","
                    "\"monitor\"  : \"%s\","
                    "\"startHz\"  : %.0f,"
                    "\"startDbm\" : %.3f,"
                    "\"stopHz\"   : %.0f,"
                    "\"stopDbm\"  : %.3f,"
                    "\"stepHz\"   : %.0f,"
                    "\"stepDbm\"  : %.3f,"
                    "\"dwellMs\"  : %d,"
                    "\"savecfg\"  : \"OFF\","
                    "\"swreset\"  : \"OFF\","
                    "\"hwreset\"  : \"OFF\","
                    "\"shutdown\" : \"OFF\" "
                    " }",
                    mRun?"ON":"OFF",            // run
                    mLockStatus?"LOCK":"FAIL",  // lock
                    timeStr,                    // time
                    mActHz,                     // actHz
                    mActDbm,                    // actDbm
                    mSweep?  "ON":"OFF",        // sweep
                    mMonitor?"ON":"OFF",        // monitor
                    mStartHz,                   // startHz
                    mStartDbm,                  // startDbm
                    mStopHz,                    // stopHz
                    mStopDbm,                   // stopDbm
                    mStepHz,                    // stepHz
                    mStepDbm,                   // stepDbm
                    mDwellMs                    // dwellMs
    );
    return(0);
}

/**
 * This internal method conducts a single step in a sweep.  As part
 * of a sweep step the leve, frequency, and lock status are implicitly 
 * updated.  It checks for sweep resent and resets for both ascending
 * and descending steps.
 */
void HwModel::DoSweepStep()
{
    int err;

    if( mParamChange ){
        mTargetHz  = mStartHz;
        mTargetDbm = mStartDbm;
        mParamChange= 0;
    }

    if( mLog&HWDEVS_LOG_SWEEP ){
       printf("step: %f Hz %f dBm\n",mTargetHz, mTargetDbm );
    }

    err = SetOutput( mTargetHz, mTargetDbm );
    if( err && mLog&HWDEVS_LOG_SWEEP ){
        fprintf(stderr,"SetupOutput err = %d\n",err);
    }

    mTargetHz  += mStepHz; 
    if( ((mStepHz>0) && (mTargetHz > mStopHz)) ||
        ((mStepHz<0) && (mTargetHz < mStopHz)) ){
        mTargetHz = mStartHz;
    }

    mTargetDbm += mStepDbm;
    if( ((mStepDbm>0) && (mTargetDbm > mStopDbm)) ||
        ((mStepDbm<0) && (mTargetDbm < mStopDbm)) ){
        mTargetDbm = mStartDbm;
    }
}

/**
 * This internal method waits the specified dwell time.  For no dwell time
 * it returns immediately.  For long dwell times the wait is broken into
 * increments to allow the dwell to be preempted.
 */
void HwModel::DoSweepDwellTime()
{
    // Effectively no dwell time
    if( mDwellMs <=0 ){
        return;
    }

    // Short dwells we just sleep it out
    if( mDwellMs < 1000 ){  // short dwells just sleep
        us_sleep( mDwellMs * 1000 );
        return;
    }

    // Longer dwells we break up so we don't get locked into a long dwell
    int ms = 0;
    while( (ms < mDwellMs) && !mParamChange ){
        us_sleep( 100 * 1000 );
        ms+=100;
    }
    return;
}

/**
 * This internal method dumps the internal model state to stdout
 */
void HwModel::ShowState()
{
    printf("Hz  : %f [%f %f]\n",mActHz,mStartHz,mStopHz);
    printf("dBm : %f [%f %f]\n",mActDbm,mStartDbm,mStopDbm);
    printf("step: %f Hz, %f dBm every %d mS\n",mStepHz,mStepDbm,mDwellMs);
    printf("Monitor = %d\n",mMonitor);
    printf("Sweep   = %d\n",mSweep);
}

/**
 * This internal method initializes the hardware. It must be invoked
 * prior to conducting any operations on the model.
 */
int HwModel::HwInit()
{

    // Open the I board so we can access other boards
    mIbrd = new Iboard();
    mIbrd->Open();

    // Open L board
    mg6pg  = mIbrd->AllocPort( mLport );
    mIbrd->EnablePort( mLport, 1 );
    mLbrd = new Lboard();
    mLbrd->Open( mg6pg );
    mLbrd->SetLog(0);

    // Maximize attenuation before opening synthesizer
    mLbrd->SetAttenDb( mLbrd->GetMaxAttenDb() );

    // Open M board
    mg6pg  = mIbrd->AllocPort( mMport );
    mIbrd->EnablePort( mMport, 1 );
    mMbrd = new Mboard();
    mMbrd->Open( mg6pg );
    mSyn = mMbrd->GetAdf4351( 0 );
    mSyn->SetLog( 0x0 );
    mSyn->SetAuxEnable( 0 );
    mSyn->SetMtld( 1 );

    return(0);
}

void HwModel::HwStop()
{
    free( mSyn );

    free( mMbrd );

    free( mLbrd );

    // Make sure devices are powered down 
    mIbrd->EnablePort( mLport, 0 );
    mIbrd->EnablePort( mMport, 0 );

    free( mIbrd );
}

/**
 * This internal method obtains a power output estimate from the L board.
 * It discards the first estimate (previous estimate within device) and
 * averages over the specified number of estimates returning that average.
 * The readings are taken as quickly as possible (and are only gated
 * by the spi speed configuration of the device).
 */
double 
HwModel::GetPwrEst( int nEst )
{
    int    cnt;
    double sum;

    mLbrd->GetPwrDbm(); // First estimate is stale
    sum = 0;
    for(cnt=0;cnt<nEst;cnt++){
       sum+=mLbrd->GetPwrDbm( );
    }
    return( sum/nEst );
}

/**
 * This internal method sets the output to the specified frequency and
 * power level.  The results of final frequency, level and lock status
 * within the model are updated.
 *
 * The approach taken uses an iterative technique to "come up" to the
 * specified level.  Low output is configured and it is raised as close
 * to the specified level as possible by iteratively reducing attenuation
 * and increasing synthesizer output power.
 */
int
HwModel::SetOutput( double targetHz, double targetDbm )
{
    int    mSynPwr;
    double attenDb;
    double dbm,delta;

    if( mLog&HWDEVS_LOG_SETOUTPUT ){
        printf("TargetHz=%f, TargetDbm=%f\n",targetHz,targetDbm);
    }

    mSynPwr  = 0;
    attenDb  = mLbrd->GetMaxAttenDb();
    mLbrd->SetAttenDb( attenDb );
    mSyn->SetMainPower( mSynPwr );
    mActHz = (double)( 
                     // Wait for 20,000 uS for lock in increments of 100uS
                     mSyn->SetFrequencyWithLock((long long)targetHz,100,20000 ) 
                     );

    mLockStatus =  mSyn->GetLock();

    mSynPwr = 0;
    delta   = 100;
    while( mSynPwr < 4 ){ // TODO adf4351 should export min/max power method
       attenDb  = mLbrd->GetMaxAttenDb();
       mLbrd->SetAttenDb( attenDb );
       mSyn->SetMainPower( mSynPwr );

       // Iterate on atten until we are within atten setting or too many
       // passes or until our attenuation goes negative
       int pass;
       pass = 0;
       dbm  = -100;
       while( ( delta*delta ) > (0.25*0.25)  && 
              (attenDb>=0)                   && 
              (attenDb<=mLbrd->GetMaxAttenDb())  &&
              pass<10  ){
          mLbrd->SetAttenDb( attenDb );
          us_sleep( 5000 );
          dbm   = GetPwrEst(mNest);
          delta = dbm - targetDbm;
          if( mLog&HWDEVS_LOG_SETOUTPUT ){
              printf("LOG: tgt=%f, dbm=%f, sp=%d, atn=%f, del=%f\n",
   		        targetDbm, dbm, mSynPwr, attenDb, delta);
          }
          attenDb += ( delta / 2 );
          if( attenDb < 0 ) attenDb = 0;
          pass++;
       }
       mActDbm = dbm;

       // Output power ok - just return with estimate
       if( (delta*delta)<(0.25*0.25) ){
           return(0);
       }

       // Output still too low, make another pass with higher power
       mSynPwr++;
       if( mLog&HWDEVS_LOG_SETOUTPUT ){
          printf("LOG: Increasing syn power to %d\n",mSynPwr);
       }
    }

    return(-1);
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
   char                 rstr[1024];
 
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
      printf("setstate v=\'%s\'\n",vstr);

      inStr = strdup( vstr );
      
      // NOTE: this doesn't parse a true JSON object
      // URL is expected to be setstate?v=name:value
      name  = strtok_r( inStr, ":", &end );
      value = strtok_r( NULL, "",   &end );
      printf("setstate name=\'%s\',value=\'%s\'\n",name,value);

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
"This utility implement a simple http based level controlled synthesizer\n"
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
