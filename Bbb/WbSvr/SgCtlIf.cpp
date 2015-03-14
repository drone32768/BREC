#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "SgCtlIf.h"

//------------------------------------------------------------------------------
static long gSgCfgHz      = 10750000;
static int  gSgCfgAttenDb = 0;
static int  gSgCfgMode    = 1;

void SgCtlIf::Configure()
{
   long long   ifHz = 1575000000;
   long long   lo0,lo1;
   long long   dHz;


   Dp()->Lo(0)->SetLog(0);
   Dp()->Lo(1)->SetLog(0);

   switch( gSgCfgMode ){
      case 0:
         printf("SgConfigure: B=%ld\n",gSgCfgHz);
         Dp()->Lo(0)->SetMainPower( 0 );
         Dp()->Lo(0)->SetAuxPower(  0 );
         Dp()->Lo(0)->SetAuxEnable( 1 );
         Dp()->Lo(0)->SetFrequency( gSgCfgHz );
         break;
      case 1:
         printf("SgConfigure: C=%ld\n",gSgCfgHz);
         Dp()->Lo(1)->SetMainPower( 0 );
         Dp()->Lo(1)->SetAuxPower(  0 );
         Dp()->Lo(1)->SetAuxEnable( 1 );
         Dp()->Lo(1)->SetFrequency( gSgCfgHz );
         break;
      case 2:
      default:
         // approx as 1 dB/MHz
         dHz  = gSgCfgAttenDb;
         lo0 = ifHz + dHz;
         lo1 = ifHz + dHz + gSgCfgHz;
         printf("SgConfigure: %lld %lld %lld\n",lo0,lo1,dHz);
         Dp()->Lo(0)->SetAuxEnable( 0 );
         Dp()->Lo(1)->SetAuxEnable( 0 );
         Dp()->Lo(0)->SetMainPower( 0 );
         Dp()->Lo(1)->SetMainPower( 0 );
         Dp()->Lo(0)->SetFrequency( lo0 );
         Dp()->Lo(1)->SetFrequency( lo1 );
         break;
   }
}

//------------------------------------------------------------------------------
SgCtlIf::SgCtlIf()
{
    mThreadExit = 0;
    mIpPort     = 0;
    mRun        = 0;
}

//------------------------------------------------------------------------------
void SgCtlIf::SetSvcPort( int port )
{
    mIpPort = port;
}

//------------------------------------------------------------------------------
static const char *helpStr=
"help\n"
"  This command\n"
"sg-syn-lock-status <N>\n"
"  Return the lock status of synthesizer N=[0..1], 0=no lock, 1=lock\n"
"sg-hz   <M>\n"
"  Set output frequency to M Hertz\n"
"sg-adb  <M>\n"
"  Set output attenuation to M dB (approximately)\n"
"sg-get-lo-hz  <M>\n"
"  Get LO M freqency in Hz\n"
"sg-power-enable  <M> <N>\n"
"  Set power enable for LO M to N=0..1\n"
"sg-mode <N>\n"
"  Set mode N=0,1,2,3\n"
" \n"
;

//------------------------------------------------------------------------------
void SgCtlIf::SvcErr( TcpSvrCon *tsc )
{
    char           lineBf[128];
    sprintf( lineBf,"err, arguments, see help\n");
    TcpSvrWrite(tsc,lineBf,strlen(lineBf));
}

//------------------------------------------------------------------------------
void SgCtlIf::SvcCmd( TcpSvrCon *tsc, Cli *cli )
{
    char          *cmdStr;
    const char    *rspStr;
    char           *arg1,*arg2;
    char           lineBf[128];
    int            dn;

    cmdStr = CliArgByIndex( cli, 0 );
    printf("Cmd= %s\n",cmdStr);        

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"help")  ){
        rspStr=helpStr;
        TcpSvrWrite(tsc,rspStr,strlen(rspStr));
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"sg-lock-status")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }

        dn   = atoi(arg1);
        if( dn<0 || dn>1 ) dn=0;

        sprintf( lineBf,"%d\n", Dp()->Lo(dn)->GetLock() ); 
        TcpSvrWrite(tsc,lineBf,strlen(lineBf));
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"sg-hz")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }

        gSgCfgHz = atol( arg1 );      
        Configure();
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"sg-adb")  ){
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }

        gSgCfgAttenDb = atol( arg1 );
        Configure();
        return;
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"sg-get-lo-hz")  ){
        int dn;
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }

        dn = (atoi( arg1 ))&1;      
        sprintf( lineBf,"%lld\n", Dp()->Lo(dn)->GetFrequency() ); 
        TcpSvrWrite(tsc,lineBf,strlen(lineBf));
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"sg-mode")  ){
        int dn;
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }

        gSgCfgMode = atol( arg1 );
        Configure();
    }

    //////////////////////////////////////// 
    if( 0==strcmp(cmdStr,"sg-power-enable")  ){
        int dn;
        arg1 = CliArgByIndex( cli, 1 );
        if( !arg1 ) { SvcErr(tsc); return; }
        dn = (atoi( arg1 ))&1;      

        int pwe;
        arg2 = CliArgByIndex( cli, 2 );
        if( !arg1 ) { SvcErr(tsc); return; }
        pwe = (atoi( arg2 ))&1;      

        Dp()->Lo(dn)->SetPowerDown( !pwe );
    }

    return;
}

//------------------------------------------------------------------------------
void SgCtlIf::RcvEvent( char *evtStr ) 
{
    printf("SgCtlIf : rcv <%s>\n",evtStr);
}

//------------------------------------------------------------------------------
void SgCtlIf::Main()
{
    int              err, rb, idx;
    char             peerStr[128];
    char             bf[128];
    Cli              *cli;
    TcpSvrCon        *tsc;

    printf("SgCtlIf:Entering main control loop\n");
    while( !mThreadExit ){

        printf("SgCtlIf:Allocate new control connect\n");
        tsc = TcpSvrConNew();
        if( !tsc ) return;

        printf("SgCtlIf:Waiting for cli control connection p=%d\n",mIpPort);
        err = TcpSvrConWaitNewCon(tsc,mIpPort);
        if( err<0 ) continue;

        TcpSvrGetClnIpStr( tsc, peerStr, sizeof(peerStr) );
        printf("SgCtlIf:Connection from %s\n",peerStr);

        printf("SgCtlIf:Servicing connection\n");
        cli = CliNew();
        if( !cli ) break;

        rb = 1;
        while( rb>0 ){
            
            printf("SgCtlIf:Waiting/read command data\n");
            rb = TcpSvrRead( tsc, bf, sizeof(bf) );
            if( rb<=0 ){
               break;
            }

            if( rb>0 ) bf[rb]=0;
    	    for( idx=0; idx<rb; idx++ ){
                err = CliInputCh( cli, bf[idx] );
                if( err > 0 ){
                    printf("SgCtlIf:Servicing command\n");
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

