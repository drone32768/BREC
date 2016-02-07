#ifndef __SG_CTLIF_H__
#define __SG_CTLIF_H__

#include "../Util/mcf.h"
#include "../Util/net.h"
#include "../Util/cli.h"
#include "Devs.h"

class SgCtlIf : public McF {

  private:
    int          mThreadExit;
    int          mIpPort;
    int          mRun;
    void         SvcErr( TcpSvrCon *tsc );

    // Required inteface elements for mcf
    void         SvcCmd( TcpSvrCon *tsc, Cli *cli );

  public:

    // Required inteface elements for mcf
    SgCtlIf();
    void         Main();
    void         RcvEvent( char *evtStr );
    void         SetSvcPort( int port );
    void         Configure( );
};

#endif
