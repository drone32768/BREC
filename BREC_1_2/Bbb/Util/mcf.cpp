#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "mcf.h"

//////////////////////////////////////////////////////////////////////////////
/// Misc /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/*
 * This routine takes a string representing a frequency in Hertz and
 * returns the double representation of the string.  Suffixes of k,m,g are
 * supported (not case sensitive).
 */
double atof_suffix( const char *str )
{
    char *nstr;
    int   len;
    float mul;
    double freqHz;

    nstr = strdup( str );

    mul = 1.0;
    len = strlen( nstr );
    switch( nstr[len-1] ){
        case 'k' :
        case 'K' :
            mul = 1e3;
	    nstr[len-1] = 0;
	    break;
        case 'm' :
        case 'M' :
            mul = 1e6;
	    nstr[len-1] = 0;
	    break;
        case 'g' :
        case 'G' :
            mul = 1e9;
	    nstr[len-1] = 0;
	    break;
        default:
	    break;
    }
    freqHz = atof( nstr );
    freqHz = freqHz * mul;
    return(freqHz);
}
/**
 * This static method takes two timeval's and returns the difference
 * in signed integer microseconds.  First arg is latest time, second is 
 * earlier.
 */
int tv_delta_useconds( struct timeval *tv2, struct timeval *tv1 )
{
    int usec;

    usec  = 1000000*(tv2->tv_sec - tv1->tv_sec);
    usec += (tv2->tv_usec - tv1->tv_usec);
    return(usec);
}

//------------------------------------------------------------------------------
/**
 * This static method takes two timeval's and returns the difference
 * in signed integer nanoseconds.  First arg is latest time, second is 
 * earlier.
 */
unsigned long long tv_delta_nseconds( struct timeval *tv2, struct timeval *tv1 )
{
    unsigned long long nsec;

    nsec  = 1000000*(tv2->tv_sec - tv1->tv_sec);   // sec  -> usec
    nsec  = nsec * 1000;                           // usec -> nsec
    nsec += 1000 * (tv2->tv_usec - tv1->tv_usec);  // residual usec -> nsec
    return(nsec);
}

//------------------------------------------------------------------------------
int us_sleep( unsigned int us )
{
    struct timespec     ts;
    struct timeval      tv1,tv2;
    unsigned int        dus;

    // Greater than threshold use nanosleep
    if( us > 10000 ){
        ts.tv_sec  = us/1000000 ;
        ts.tv_nsec = (us%1000000)*1000;
        nanosleep( &ts, NULL );
    }

    // Less than threshold spin
    else{
        dus = 0;
        gettimeofday( &tv1, NULL );
        while( dus < us ){
            gettimeofday( &tv2, NULL );
            dus = tv_delta_useconds( &tv2, &tv1 );
        }
    }

    return(0);
}

//----------------------------------------------------------------------------

// Single global even registry (does not preclude others)
static EvR *gpEvr = NULL;

/**
 * Returns a pointer to the global static event registry
 */
EvR *GetEvrSingleton()
{
    if( !gpEvr ){
        gpEvr = new EvR();
    }
    return( gpEvr );
}

//----------------------------------------------------------------------------
/**
 * Internal method to tie object main with pthread main
 */
static void *ThreadMain( void *arg )
{
    McF *mcf = (McF*)arg;
    mcf->Main();
    return( NULL );
}

//------------------------------------------------------------------------------
/**
 * This routine is given a cape string name.  It looks through the
 * sysfs based capemanager output in an effort to find an occurence of
 * this string.
 *
 * On the X65 platform it uses the file /tmp/capes.txt as the sysfs 
 * cape manager output.
 *
 * Returns: -1 err, 0 not found, 1 found.
 */
int FindCapeByName( const char *findStr )
{
    char lineBf[1024];
    int  found;
    FILE *fp;

    // Try to open the cape manager (at one of the versions)
#   ifdef TGT_X86 
    fp = fopen("/tmp/capes.txt","r");
#   else
    int idx = 0;
    while( idx<20 ){
       snprintf(lineBf, sizeof(lineBf), 
                "/sys/devices/bone_capemgr.%d/slots", idx);
       fp = fopen(lineBf,"r");
       if( fp!=NULL ) break;
       idx++;
    }
#   endif

    // Could not open cape manager
    if( !fp ){
        return( -1 );
    }

    // Read each of the cape lines
    found = 0;
    while( fgets(lineBf,sizeof(lineBf),fp) ){

        // If this line contains the sought after name break out of search
        if( strstr( lineBf, findStr ) ){
            found = 1;
            break;
        }

    }

    // Close capge manager file handle
    fclose(fp);

    // Return the results
    return( found );
}

//////////////////////////////////////////////////////////////////////////////
/// Other ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
StopWatch::StopWatch()
{
    mIdx = 0;
}

void StopWatch::Start()
{
    gettimeofday( &tv1, NULL );
}

void StopWatch::Stop()
{
    gettimeofday( &tv2, NULL );
}

unsigned long StopWatch::GetuS()
{
    unsigned long us;
    us  = (tv2.tv_sec  - tv1.tv_sec) * 1000;
    us += (tv2.tv_usec - tv1.tv_usec);
    return( us );
}

void StopWatch::StopWithHistory()
{
    Stop();
    mHistory[ mIdx ] = GetuS();
    mIdx = (mIdx+1)%STOP_WATCH_HISTORY_LEN;
}

static int uint_cmp( const void *a1, const void *a2 )
{
    unsigned int *v1 = (unsigned int*)a1;
    unsigned int *v2 = (unsigned int*)a2;

    if( *v1 <  *v2 ) return( -1 );
    if( *v1 == *v2 ) return( 0 );
    return( 1 );
}

unsigned long StopWatch::MedianuS()
{
    qsort( mHistory, sizeof(unsigned long), STOP_WATCH_HISTORY_LEN, uint_cmp );
    return( mHistory[ STOP_WATCH_HISTORY_LEN / 2 ] );
}

//////////////////////////////////////////////////////////////////////////////
/// SimpleMutex //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
SimpleMutex::SimpleMutex()
{
   pthread_mutex_init( &mLock, NULL );
}

int
SimpleMutex::Lock()
{
   int err;
   err = pthread_mutex_lock( &mLock );
   return( err );
}

int
SimpleMutex::Unlock()
{
   int err;
   err = pthread_mutex_unlock( &mLock );
   return( err );
}

//////////////////////////////////////////////////////////////////////////////
/// McF //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
/*
 * Constructor for a mcf.
 * - objects registry is cleared
 */
McF::McF()
{
    mEvr = NULL;
}

//----------------------------------------------------------------------------
McF::~McF()
{
}

//----------------------------------------------------------------------------
/**
 * This method starts the thread associate with this object.  The thread
 * will begin executing at the objects Main().
 * Returns: 0 on success.
 */
int McF::Start()
{
    pthread_t          thread;
    pthread_attr_t     pthread_attr;

    mThreadExit = 0;

    pthread_attr_init(&pthread_attr);
    pthread_create( &thread,
                    &pthread_attr,
                    ThreadMain, 
                    (void*)this);
    return(0);
}

//----------------------------------------------------------------------------
/**
 * This method sets the thread exit indicator for the object and returns
 * Returns: 0 on success.
 */
int McF::Stop()
{
    mThreadExit = 1;
    return(0);
}

//----------------------------------------------------------------------------
/**
 * This method sends the given event to the registry this object is
 * registered with.
 */
void McF::SndEvent( const char *evtStr )
{
    if( NULL != mEvr ){
       mEvr->Inject( this, evtStr );
    }
}


//////////////////////////////////////////////////////////////////////////////
/// EvR //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
/**
 * Constructor for event registry.
 * - All handlers are cleared.
 */
EvR::EvR()
{
    int idx;
    for(idx=0;idx<EVT_MAX_HANDLERS;idx++){
        mHandlers[idx] = NULL;
    }
  
}

//----------------------------------------------------------------------------
/**
 * This method registers the given object with this registry.
 * NOTE: This method is not MT safe.
 * NOTE: Fixed and finite number of objects can be registered in a single reg
 * NOTE: Returns 0 on success, non zero else
 */
int EvR::Register( McF *obj )
{
    int idx;
    for(idx=0;idx<EVT_MAX_HANDLERS;idx++){
        if( NULL == mHandlers[idx] ){
            mHandlers[idx] = obj;
            obj->mEvr = this;
            return(0);
        }
    }
    fprintf(stderr,"ERROR: cannot register handler\n");
    return(-1);
}

//----------------------------------------------------------------------------
/**
 * This method injects the given event into all objects registered with 
 * this registry.  The first argument is treated as the source object
 * of the event and not delivered to any object that matches it in the
 * registry. (e.g. if an object desires to get its own injected events
 * src can be NULL).
 */
void EvR::Inject( McF *src, const char *evtStr )
{
    int idx;
    char *evtStrCpy;

    for( idx=0;idx<EVT_MAX_HANDLERS;idx++){
        if( NULL != mHandlers[idx] && src!= mHandlers[idx] ){
            evtStrCpy = strdup( evtStr );
            mHandlers[idx]->RcvEvent( evtStrCpy );
            free(evtStrCpy);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#ifdef _UNIT_TEST_

class Obj1 : public McF {
    public:
    Obj1();
    void Main();
    void RcvEvent( char *evtStr ); 
};

Obj1::Obj1()
{
}

void Obj1::Main()
{
    while( 1 ) {
       printf("Obj1:Main...\n");
       sleep(1);
    }
}

void Obj1::RcvEvent( char *evtStr )
{
    printf("Obj1:RcvEvent <%s>\n",evtStr);
}

class Obj2 : public McF {
    public:
    Obj2();
    void Main();
    void RcvEvent( char *evtStr ); 
};

Obj2::Obj2()
{
}

void Obj2::Main()
{
    while( 1 ) {
       printf("Obj2:Main...\n");
       SndEvent( "obj1.generic-event" );
       sleep(1);
    }
}

void Obj2::RcvEvent( char *evtStr )
{
    printf("Obj2:RcvEvent <%s>\n",evtStr);
}

int main( int argc, char *argv[] )
{
    Obj1 *obj1;
    Obj2 *obj2;
    EvR  evr;

    obj1 = new Obj1();
    obj2 = new Obj2();

    evr.Register( obj1 );
    evr.Register( obj2 );

    obj1->Start();
    obj2->Start();

    while( 1 ) sleep(1);
}

#endif /* _UNIT_TEST_ */
