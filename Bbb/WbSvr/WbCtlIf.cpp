#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "WbCtlIf.h"
#include "WbTests.h"

#include "AdcCtlIf.h"
#include "AdcDatIf.h"
#include "AdcTcpIf.h"
#include "SiCtlIf.h"
#include "SgCtlIf.h"

//------------------------------------------------------------------------------
#define DBG_CONNECT 0x0000001
#define DBG_COMMAND 0x0000002

WbCtlIf::WbCtlIf()
{
    mThreadExit = 0;
    mIpPort     = 0;
    mRun        = 0;
    mDbgMask    = ( DBG_CONNECT );

#   define MAX_SAMPLE_SET 65536
    mSamples = (short*)malloc( MAX_SAMPLE_SET );
}

//------------------------------------------------------------------------------
void WbCtlIf::SetSvcPort( int port )
{
    mIpPort = port;
}

//------------------------------------------------------------------------------
static const char *helpStr=
"help\n"
"  This command\n"
"wb-inject <line>\n"
"  Inject <line> as an event\n"
"wb-dbg <M>\n"
"  Set debug mask to hex value M (can prefix hex with 0x)\n"
"wb-syn-lock-status <N>\n"
"  Return the lock status of synthesizer N, 0=no lock, 1=lock\n"
"wb-syn-freq-hz <N> <M>\n"
"  Set synthesizer N frequency to M Hertz\n"
"wb-syn-aux-power <N> <M>\n"
"  Set synthesizer N aux channel power output to M=[0..3]\n"
"wb-syn-aux-enable <N> <M>\n"
"  Set synthesizer N aux channel output M=1=enable, M=0=disable\n"
"wb-syn-main-power <N> <M>\n"
"  Set synthesizer N main channel output M=[0..3]\n"
"wb-syn-cpi <N> <M>\n"
"  Set the charge pump current on syn <N> to M=0..15\n"
"wb-syn-mtld <N> <M>\n"
"  Set the syn <N> mute to lock detec to M=0..1\n"
"wb-syn-power-down <N>\n"
"  Power down/up syn <N>\n"
"wb-syn-show <N>\n"
"  Show registers for syn <N>\n"
"wb-syn-low-spur <N> <M>\n"
"  Set the syn <N> spur mode to <M>=[0 | 1]\n"
"  Power down/up syn <N>\n"
"wb-get-rms <N>\n"
"  Return the current rms(dB16bFS) and mean bias value using <N> samples\n"
"wb-csps <N>\n"
"  Set the complex sample rate to N=500000 | 625000 | ....\n"
"wb-test00 <M>\n"
"wb-test01 <M>\n"
"  Execute test with parameter M\n"
"wb-rf-chnl <N>\n"
"  Set R board rf chnl [0..3]\n"
"wb-rf-atten <M>\n"
"  Set R board attenuation value to <M>\n"
"x-src <M>\n"
"  Set X board fifo source to <M>\n"
"x-pinc <M>\n"
"  Set X board phase inc to <M>\n"
" \n"
;

//------------------------------------------------------------------------------
void WbCtlIf::SvcErr( TcpSvrCon *tsc )
{
    char           lineBf[128];
    sprintf( lineBf,"err, arguments, see help\n");
    TcpSvrWrite(tsc,lineBf,strlen(lineBf));
}

//------------------------------------------------------------------------------
void WbCtlIf::SvcCmd( TcpSvrCon *tsc, Cli *cli )
{
    char          *cmdStr;
    const char    *rspStr;
    char           *arg1,*arg2,*arg3;
    char           lineBf[128];
    int            dn;

    unsigned long long llVal;
    int                intVal;

    cmdStr = CliArgByIndex( cli, 0 );
    if( mDbgMask & DBG_COMMAND ){
        printf("Cmd= %s\n",cmdStr);        
    }

    // TODO - error check arguments, lack of arguments ...

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"help")  ){
        rspStr=helpStr;
        TcpSvrWrite(tsc,rspStr,strlen(rspStr));
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"wb-inject")  ){
        arg1 = CliArgByIndex( cli, 1 );
        arg2 = CliArgByIndex( cli, 2 );
        arg3 = CliArgByIndex( cli, 3 );
        sprintf(lineBf,"%s %s %s",
                        arg1?arg1:" ",
                        arg2?arg2:" ",
                        arg3?arg3:" "  );
        SndEvent(lineBf);
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"wb-dbg")  ){
        char *end;
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }
        mDbgMask = strtol(arg1,&end,0);    
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"wb-syn-lock-status")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }

        dn   = atoi(arg1);
        if( dn<0 || dn>1 ) dn=0;

        sprintf( lineBf,"%d\n", Dp()->Lo(dn)->GetLock() ); 
        TcpSvrWrite(tsc,lineBf,strlen(lineBf));
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"wb-syn-freq-hz")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }
        dn   = atoi(arg1);
        if( dn<0 || dn>1 ) dn=0;

        arg2 = CliArgByIndex( cli, 2 );
        if( !arg2 ) { SvcErr(tsc); return; }
        llVal = atoll( arg2 );      

        Dp()->Lo(dn)->SetFrequency(  llVal );

        sprintf( lineBf,"%llu\n", Dp()->Lo(dn)->GetFrequency() ); 
        TcpSvrWrite(tsc,lineBf,strlen(lineBf));

        // printf("%d f=%llu\n",dn,Dp()->Lo(dn)->GetFrequency());

        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"wb-syn-aux-power")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }

        dn   = atoi(arg1);
        if( dn<0 || dn>1 ) dn=0;

        arg2 = CliArgByIndex( cli, 2 );
        if( !arg2 ) { SvcErr(tsc); return; }
        intVal= atoi(arg2);

        Dp()->Lo(dn)->SetAuxPower(  intVal );
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"wb-syn-aux-enable")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }
        dn   = atoi(arg1);
        if( dn<0 || dn>1 ) dn=0;

        arg2 = CliArgByIndex( cli, 2 );
        if( !arg2 ) { SvcErr(tsc); return; }
        intVal= atoi(arg2);

        Dp()->Lo(dn)->SetAuxEnable(  intVal );
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"wb-syn-main-power")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }
        dn   = atoi(arg1);
        if( dn<0 || dn>1 ) dn=0;

        arg2 = CliArgByIndex( cli, 2 );
        if( !arg2 ) { SvcErr(tsc); return; }
        intVal= atoi(arg2);

        Dp()->Lo(dn)->SetMainPower(  intVal );
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"wb-get-rms")  ){
        double mean, rms;
        int nSamples;

        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }
        nSamples= atoi(arg1);
        if( nSamples > MAX_SAMPLE_SET ) nSamples = MAX_SAMPLE_SET;

        mean = Dp()->Adc()->GetRms( nSamples, mSamples, &rms );

        sprintf( lineBf,"%f, %f\n", rms, mean );
        TcpSvrWrite(tsc,lineBf,strlen(lineBf));
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"wb-csps")  ){
        int csps;

        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }
        csps = atoi(arg1);

        Dp()->Adc()->SetComplexSampleRate( csps );
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"wb-test00")  ){
        int tn;

        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }
        tn = atoi(arg1);

        Test00(Dp()->Lo(0), Dp()->Lo(1), Dp()->Adc(), tn  );
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"wb-test01")  ){
        int tn;

        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }
        tn = atoi(arg1);

        Test01(Dp()->Lo(0), Dp()->Lo(1), Dp()->Adc(), tn  );
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"wb-syn-cpi")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }

        dn   = atoi(arg1);
        if( dn<0 || dn>1 ) dn=0;

        arg2 = CliArgByIndex( cli, 2 );
        if( !arg2 ) { SvcErr(tsc); return; }
        intVal= atoi(arg2);

        Dp()->Lo(dn)->SetCpCurrent(  intVal );
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"wb-syn-mtld")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }

        dn   = atoi(arg1);
        if( dn<0 || dn>1 ) dn=0;

        arg2 = CliArgByIndex( cli, 2 );
        if( !arg2 ) { SvcErr(tsc); return; }
        intVal= atoi(arg2);

        Dp()->Lo(dn)->SetMtld(  intVal );

        sprintf( lineBf,"%d\n", intVal );
        TcpSvrWrite(tsc,lineBf,strlen(lineBf));

        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"wb-syn-power-down")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }

        dn   = atoi(arg1);
        if( dn<0 || dn>1 ) dn=0;

        arg2 = CliArgByIndex( cli, 2 );
        if( !arg2 ) { SvcErr(tsc); return; }
        intVal= atoi(arg2);

        Dp()->Lo(dn)->SetPowerDown(  intVal );
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"wb-syn-show")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }

        dn   = atoi(arg1);
        if( dn<0 || dn>1 ) dn=0;

        Dp()->Lo(dn)->Show();
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"wb-syn-low-spur")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }

        dn   = atoi(arg1);
        if( dn<0 || dn>1 ) dn=0;

        arg2 = CliArgByIndex( cli, 2 );
        if( !arg2 ) { SvcErr(tsc); return; }
        intVal= atoi(arg2);

        Dp()->Lo(dn)->SetLowSpurMode(  intVal );
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"wb-rf-chnl")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }
        intVal= atoi(arg1);

	printf("Setting channel %d\n",intVal);
        Dp()->Rbrd()->SetChannel(  intVal );
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"wb-rf-atten")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }
        intVal= atoi(arg1);

	printf("Setting atten %d\n",intVal);
        Dp()->Rbrd()->SetAtten(  intVal );
        return;
    }

    if( 0==strcmp(cmdStr,"x-src")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }
        intVal= atoi(arg1);

        Xboard *xBrd;
        xBrd = (Xboard*)( Dp()->Adc() );
        xBrd->SetSource(  intVal );
    }

    if( 0==strcmp(cmdStr,"x-pinc")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }
        intVal= atoi(arg1);

        Xboard *xBrd;
        xBrd = (Xboard*)( Dp()->Adc() );
        xBrd->SetLoFreq(  intVal );
    }
    return;
}

//------------------------------------------------------------------------------
void WbCtlIf::RcvEvent( char *evtStr ) 
{
    printf("WbCtlIf : rcv <%s>\n",evtStr);
}

//------------------------------------------------------------------------------
void WbCtlIf::Main()
{
    int              err, rb, idx;
    char             peerStr[128];
    char             bf[128];
    Cli              *cli;
    TcpSvrCon        *tsc;
    AdcCtlIf         *adcCtl;
    AdcDatIf         *adcDat;
    AdcTcpIf         *adcTcp;
    SgCtlIf          *sgCtl;
    SiCtlIf          *siCtl;

    // Start devices
    Dp()->Open();

    printf("============ WbCtlIf:Starting ADC network interface ===========\n");

    // Create a udp version
    adcCtl = new AdcCtlIf();
    adcDat = new AdcDatIf();
    adcCtl->SetSvcPort( 6787 );
    GetEvrSingleton()->Register( adcCtl );
    GetEvrSingleton()->Register( adcDat );
    adcCtl->Start();
    adcDat->Start();

    // Create a tcp version
    adcTcp = new AdcTcpIf();
    adcTcp->SetSvcPort( 6788 );
    adcTcp->Start();

    printf("============ WbCtlIf:Starting SI network interface ============\n");
    siCtl = new SiCtlIf();
    siCtl->SetSvcPort( 6789 );
    GetEvrSingleton()->Register( siCtl );
    siCtl->Start();

    printf("============ WbCtlIf:Starting SG network interface ============\n");
    sgCtl = new SgCtlIf();
    sgCtl->SetSvcPort( 6786 );
    GetEvrSingleton()->Register( sgCtl );
    sgCtl->Start();

    printf("============ WbCtlIf:Starting WB network interface ============\n");

    printf("WbCtlIf:Entering main control loop\n");
    while( !mThreadExit ){

        if(mDbgMask&DBG_CONNECT){
           printf("WbCtlIf:Allocate new control connect\n");
        }
        tsc = TcpSvrConNew();
        if( !tsc ) return;

        if(mDbgMask&DBG_CONNECT){
           printf("WbCtlIf:Waiting for cli control connection p=%d\n",mIpPort);
        }
        err = TcpSvrConWaitNewCon(tsc,mIpPort);
        if( err<0 ) continue;

        TcpSvrGetClnIpStr( tsc, peerStr, sizeof(peerStr) );
        if(mDbgMask&DBG_CONNECT){
           printf("Connection from %s\n",peerStr);
        }

        if(mDbgMask&DBG_CONNECT){
           printf("WbCtlIf:Servicing connection\n");
        }
        cli = CliNew();
        if( !cli ) break;

        rb = 1;
        while( rb>0 ){
            
            if( mDbgMask & DBG_COMMAND ){
                printf("WbCtlIf:Waiting/read command data\n");
            }
            rb = TcpSvrRead( tsc, bf, sizeof(bf)-1 );
            if( rb<=0 ){
               break;
            }

            if( rb>0 ) bf[rb]=0;
    	    for( idx=0; idx<rb; idx++ ){
                err = CliInputCh( cli, bf[idx] );
                if( err > 0 ){
                    if( mDbgMask & DBG_COMMAND ){
                        printf("WbCtlIf:Servicing command\n");
                    }
                    SvcCmd( tsc, cli );
                    CliReset( cli );
                }
                if( err < 0 ){
                    break;
                }
            }		 

        }

        TcpSvrClose( tsc );
        free( tsc );
        free( cli );
    }
}
