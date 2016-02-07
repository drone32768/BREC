#ifndef __WB_CTLIF_H__
#define __WB_CTLIF_H__

#include "../Util/mcf.h"
#include "../Util/net.h"
#include "../Util/cli.h"
#include "Devs.h"

class WbCtlIf : public McF {

  private:
    int           mThreadExit;
    int           mIpPort;
    int           mRun;
    unsigned int  mDbgMask;

    short        *mSamples;

    void         SvcErr( TcpSvrCon *tsc );

    // Required inteface elements for mcf
    void         SvcCmd( TcpSvrCon *tsc, Cli *cli );

  public:

    // Required inteface elements for mcf
    WbCtlIf();
    void         Main();
    void         RcvEvent( char *evtStr );
    void         SetSvcPort( int port );
};

#endif
