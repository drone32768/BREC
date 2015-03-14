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

#include "net.h"

/*
Without "netstat -g"  do the following

cat /proc/net/dev_mcast

cat /proc/net/igmp
--> show groups

*/

////////////////////////////////////////////////////////////////////////////
McgCln *McgClnNew( char *mcIpStr, int mcIpPort )
{
    McgCln *mcc;
    int     opt;
    int     err;
    struct sockaddr_in localAddr;

    mcc = (McgCln*)malloc( sizeof(McgCln) );
    if(!mcc) return(NULL);

    mcc->sockFd = socket(AF_INET, SOCK_DGRAM, 0 );
    if( mcc->sockFd < 0 ){
         perror("socket");
         free( mcc );
         return(NULL);
    }

    opt = 1;
    setsockopt( mcc->sockFd, 
                SOL_SOCKET, 
                SO_REUSEADDR,
                (char*)&opt,
                sizeof(opt)
              );
 
     memset( (char*)&localAddr, 0, sizeof(localAddr) );
     localAddr.sin_family      = AF_INET;
     localAddr.sin_port        = htons(mcIpPort);
     localAddr.sin_addr.s_addr = INADDR_ANY; 

     err = bind( mcc->sockFd, (struct sockaddr*)&localAddr, sizeof(localAddr) );
     if( err ) {
         perror("bind");
         close( mcc->sockFd );
         free( mcc );
         return( NULL );
     }
   
     mcc->grpAddr.imr_multiaddr.s_addr = inet_addr(mcIpStr);
     /* examples show this, however, its not clear its strictly required
     mcc->grpAddr.imr_interface.s_addr = inet_addr("192.168.0.2");
     */   
     err = setsockopt( mcc->sockFd, 
                       IPPROTO_IP, 
                       IP_ADD_MEMBERSHIP,
                       (char*)&(mcc->grpAddr),
                       sizeof(mcc->grpAddr)
                     );
      if( err<0 ) {
          perror("mc group join");
          close(mcc->sockFd);
          free(mcc);
          return(NULL);
      }

      return(mcc);
}

////////////////////////////////////////////////////////////////////////////
int McgClnRead( McgCln *mcc, char *bf, int nbytes )
{
    int nb;
    nb = read( mcc->sockFd, bf, nbytes );
    return( nb );
}

////////////////////////////////////////////////////////////////////////////
void McgClnDelete( McgCln *mcc )
{
    close(mcc->sockFd);
    free(mcc);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
McgSvr *McgSvrNew( const char *mcIpStr, int mcIpPort )
{
    McgSvr            *mcs;
    char               bopt;

    mcs = (McgSvr*)malloc( sizeof(McgSvr) );
    if(!mcs) return(NULL);

    mcs->sockFd = socket(AF_INET, SOCK_DGRAM, 0 );
    if( mcs->sockFd < 0 ){
         perror("socket");
         free( mcs );
         return(NULL);
    }

    memset( (char*)&(mcs->grpAddr), 0, sizeof(mcs->grpAddr) );
    mcs->grpAddr.sin_family      = AF_INET;
    mcs->grpAddr.sin_addr.s_addr = inet_addr( mcIpStr );
    mcs->grpAddr.sin_port        = htons( mcIpPort );

    // Enable multicast loop back to this host
    bopt = 1;
    setsockopt(mcs->sockFd,IPPROTO_IP, IP_MULTICAST_LOOP, &bopt, sizeof(bopt) );

    /* examples show this, however, its not clear its strictly required
    localInterface.s_addr = inet_addr("192.168.0.2");
    ret = setsockopt(mcs->sockFd, IPPROTO_IP, IP_MULTICAST_IF,
		    (char*)&localInterface,
		    sizeof(localInterface) );
    if( ret < 0 ){
        perror("local if");
        free(mcs);
        return(NULL);
    }
    */
      
    return(mcs);
}

////////////////////////////////////////////////////////////////////////////
int McgSvrSend( McgSvr *mcs, unsigned char *sndBf, int sndBytes )
{
    int nbytes;
    nbytes = sendto( mcs->sockFd, 
                  sndBf, 
                  sndBytes, 
                  0, 
                  (struct sockaddr*)&(mcs->grpAddr),
                  sizeof(mcs->grpAddr) 
                );
    return(nbytes);
}

////////////////////////////////////////////////////////////////////////////
void McgSvrDelete( McgSvr *mcs )
{
    close( mcs->sockFd );
    free( mcs );
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
TcpClnCon *TcpClnConNew()
{
    TcpClnCon *tcc;

    tcc = (TcpClnCon*)malloc( sizeof(TcpClnCon) );
    if( !tcc ) return( NULL );

    tcc->sockFd = -1;

    return( tcc );
}

////////////////////////////////////////////////////////////////////////////
void TcpClnConDelete( TcpClnCon *tcc )
{
    if( tcc->sockFd > 0 ){
        close( tcc->sockFd );
    }
    free( tcc );
}

////////////////////////////////////////////////////////////////////////////
int TcpClnConnect( 
   TcpClnCon  *tcc, 
   const char *svcIpStr, 
   int         svcPort, 
   int         msTimeout )
{
    int                err;
    struct sockaddr_in svrAddr;
    int                nReady;
    fd_set             fdset;
    struct timeval     tv;

    // Create a socket
    tcc->sockFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if( tcc->sockFd < 0){
        perror("socket error");
        return( -1 );
    }

    // Setup destination/server address
    memset( &svrAddr, 0 , sizeof(svrAddr) );
    svrAddr.sin_family      = AF_INET;
    svrAddr.sin_addr.s_addr = inet_addr( svcIpStr );
    svrAddr.sin_port        = htons(svcPort);

    // Set to non blocking so we can timeout on operations
    fcntl( tcc->sockFd, F_SETFL, O_NONBLOCK );

    /*
     * NOTE: There are several subtle aspects to non blocking connects. Major
     * cases are:
     * a) host is present/accessible with svc port open
     * b) host is present/accessible with svc port NOT open
     * c) host is NOT present or reachable
     */

    // Initiate the connect
    err = connect( tcc->sockFd, (struct sockaddr *)&svrAddr, sizeof(svrAddr) );
    if( (-1==err) && (EINPROGRESS!=errno) ){
        perror("connect error");
        return( -1 );
    }
    // perror("connect errno is:");

    // Do a select for up to the specified timeout waiting for a connection
    FD_ZERO( &fdset );
    FD_SET( tcc->sockFd, &fdset );
    tv.tv_sec = 0;
    tv.tv_usec= msTimeout * 1000;
    nReady = select( (tcc->sockFd+1), NULL, &fdset, NULL, &tv );

    // printf("nReady=%d fdisset=%d\n",nReady,FD_ISSET(tcc->sockFd,&fdset));

    // If the connect did not succeed clean and return error
    if( (-1==nReady) || (!FD_ISSET(tcc->sockFd, &fdset)) ){
        perror("failed select on connect attempt");
        close( tcc->sockFd );
        return(-1);
    }

    // Connected
    return( 0 );
}

////////////////////////////////////////////////////////////////////////////
int TcpClnSend( TcpClnCon *tcc, unsigned char *sndBf, int sndBytes )
{
    int nbytes;

    nbytes = send( tcc->sockFd, sndBf, sndBytes, 0 );
    return( nbytes );
}

////////////////////////////////////////////////////////////////////////////
int TcpClnRecv( TcpClnCon *tcc, unsigned char *rcvBf, int rcvBytes )
{
    int nbytes;

    nbytes = recv( tcc->sockFd, rcvBf, rcvBytes, 0 );
    return( nbytes );
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
TcpSvrCon *TcpSvrConNew()
{
    TcpSvrCon *tsc;

    tsc = (TcpSvrCon*)malloc( sizeof(TcpSvrCon) );
    if(!tsc) return(NULL);

    return( tsc );
}

int TcpSvrConBind( TcpSvrCon *tsc, int svcPort )
{
    int                err;
    struct sockaddr_in svrAddr;
    int                optval;

    tsc->sockFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if( tsc->sockFd < 0){
        perror("socket error");
        return( -1 );
    }

    optval = 1;
    setsockopt(tsc->sockFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    memset( &svrAddr, 0 , sizeof(svrAddr) );
    svrAddr.sin_family      = AF_INET;
    svrAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    svrAddr.sin_port        = htons(svcPort);

    err = bind(tsc->sockFd, (struct sockaddr *)&svrAddr,sizeof(svrAddr) );
    if( err<0 ){
        perror("bind error");
        close( tsc->sockFd );
        return( -2 );
    }

    err = listen(tsc->sockFd,0);
    if( err<0 ){
        close( tsc->sockFd );
        perror("listen error");
        return( -3 );
    }
    return(0);
}

int TcpSvrConAccept( TcpSvrCon *tsc )
{
    int                err;
    int                addrBytes;
    int                clnFd;
    struct sockaddr_in clnAddr;
    socklen_t          addrLen;

    addrBytes = sizeof(clnAddr);
    clnFd = accept( tsc->sockFd, 
                    (struct sockaddr*)&clnAddr, 
                    (socklen_t*)&addrBytes );
    if( clnFd<0 ){
        close( tsc->sockFd );
        perror("accept error");
        return( -3 );
    }

    close ( tsc->sockFd );
    tsc->sockFd = clnFd;

    addrLen = sizeof( tsc->peerAddr );
    err = getpeername( tsc->sockFd, (sockaddr*)&(tsc->peerAddr), &addrLen );
    if( err ) {
        perror("getpeername");
    }

    return(0);
}

int TcpSvrConWaitNewCon( TcpSvrCon *tsc, int svcPort ) 
{
   if( TcpSvrConBind( tsc, svcPort ) ) return(-1);
   if( TcpSvrConAccept( tsc        ) ) return(-2);
   return( 0 );
}

int TcpSvrGetClnIpStr( TcpSvrCon *tsc, char *strOut, int strLen )
{
    inet_ntop( AF_INET, 
                     &(tsc->peerAddr.sin_addr),
                     strOut,
                     strLen );

    return( 0 );
}

int TcpSvrGetSvcIpStr( TcpSvrCon *tsc, char *strOut, int strLen )
{
    int                err;
    int                addrBytes;
    struct sockaddr_in svrAddr;

    addrBytes = sizeof(svrAddr);
    err = getsockname( tsc->sockFd, 
                 (struct sockaddr*)&svrAddr,
                 (socklen_t*)&addrBytes);
    if( err<0 ){
       perror("TcpSvrGetSvcIpStr");
    }

    inet_ntop( AF_INET, 
                     &(svrAddr.sin_addr),
                     strOut,
                     strLen );

    return(0);
}


int TcpSvrRead( TcpSvrCon *tsc, char *bf, int nbytes )
{
    int nb;
    nb = recv( tsc->sockFd, bf, nbytes, 0 );
    if( nb<=0 ){
       perror("tcp read");
    }
    return( nb );
}

int TcpSvrReadReady( TcpSvrCon *tsc )
{
    fd_set         rfds;
    struct timeval tv;
    int            nRdy;
  
    FD_ZERO( &rfds );
    FD_SET( tsc->sockFd, &rfds );
    tv.tv_sec  = 0;
    tv.tv_usec = 0;
    nRdy = select( (tsc->sockFd + 1), &rfds, NULL, NULL, &tv );

    return( nRdy );
}

int TcpSvrWrite( TcpSvrCon *tsc, const char *bf, int nbytes )
{
    int nb;

    nb = write( tsc->sockFd, bf, nbytes );
    if( nb!=nbytes ){
        perror("tcp write");
    }
    return(nb);
}

void TcpSvrClose( TcpSvrCon *tsc )
{
    close( tsc->sockFd );
    tsc->sockFd = 0;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
UdpSvr *UdpSvrNew( int svcPort, int rcvBcast )
{
    UdpSvr            *usv;
    int                bopt;
    int                ret;
    struct sockaddr_in svrAddr;

    usv = (UdpSvr*)malloc( sizeof(UdpSvr) );
    if(!usv) return(NULL);

    usv->sockFd = socket(AF_INET, SOCK_DGRAM, 0 );
    if( usv->sockFd < 0 ){
         perror("socket");
         free( usv );
         return(NULL);
    }

    // Enable broadcast 
    if( rcvBcast ){
        bopt = 1;
        ret=setsockopt(usv->sockFd,
                       SOL_SOCKET, SO_BROADCAST, &bopt, sizeof(bopt) );
        if( ret<0 ){
           perror("UdpSvrNew:setsockopt(SO_BROADCAST)");
        }
    }

    memset( &svrAddr, 0 , sizeof(svrAddr) );
    svrAddr.sin_family      = AF_INET;
    svrAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    svrAddr.sin_port        = htons(svcPort);

    ret = bind(usv->sockFd, (struct sockaddr *)&svrAddr,sizeof(svrAddr) );
    if( ret<0 ){
        perror("bind error");
        close( usv->sockFd );
        return( NULL);
    }

    return(usv);
}

////////////////////////////////////////////////////////////////////////////
int UdpSvrSendto( 
   UdpSvr        *usv, 
   const char    *dstIpStr, 
   int            dstPort, 
   unsigned char *sndBf, 
   int            sndBytes )
{
    int                nbytes;
    struct sockaddr_in dstAddr;

    memset( &dstAddr, 0 , sizeof(dstAddr) );
    dstAddr.sin_family      = AF_INET;
    dstAddr.sin_addr.s_addr = inet_addr( dstIpStr );
    dstAddr.sin_port        = htons(dstPort);

    nbytes = sendto( usv->sockFd, 
                  sndBf, 
                  sndBytes, 
                  0, 
                  (struct sockaddr*)&(dstAddr), 
                  sizeof(dstAddr) 
                );
    if( nbytes<0 ){
        perror("UdpSvrSendto");
    }

    return(nbytes);
}

////////////////////////////////////////////////////////////////////////////
int UdpSvrRecv( UdpSvr *usv, unsigned char *rcvBf, int rcvBytes )
{
    int nbytes;
    socklen_t addrlen;

    addrlen = sizeof(usv->lastRcvAddr);
    nbytes = recvfrom( usv->sockFd,
                       rcvBf,
                       rcvBytes,
                       0,
                       (struct sockaddr*)&usv->lastRcvAddr,
                       &addrlen);
    return(nbytes);
}

////////////////////////////////////////////////////////////////////////////
void UdpSvrDelete( UdpSvr *usv )
{
    close( usv->sockFd );
    free( usv );
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
UdpCln *UdpClnNew( )
{
    UdpCln            *ucl;

    ucl = (UdpCln*)malloc( sizeof(UdpCln) );
    if(!ucl) return(NULL);

    ucl->sockFd = socket(AF_INET, SOCK_DGRAM, 0 );
    if( ucl->sockFd < 0 ){
         perror("socket");
         free( ucl );
         return(NULL);
    }
    memset( &ucl->dstAddr, 0 , sizeof(ucl->dstAddr) );
    return(ucl);
}

////////////////////////////////////////////////////////////////////////////
void UdpClnSetDst( UdpCln *ucl, char *dstIpStr, int dstPort )
{
    memset( &ucl->dstAddr, 0 , sizeof(ucl->dstAddr) );
    ucl->dstAddr.sin_family      = AF_INET;
    ucl->dstAddr.sin_addr.s_addr = inet_addr(dstIpStr);
    ucl->dstAddr.sin_port        = htons(dstPort);
}

////////////////////////////////////////////////////////////////////////////
int UdpClnSend( UdpCln *ucl, unsigned char *sndBf, int sndBytes )
{
    int nbytes;

    nbytes = sendto( ucl->sockFd, 
                  sndBf, 
                  sndBytes, 
                  0, 
                  (struct sockaddr*)&(ucl->dstAddr),
                  sizeof(ucl->dstAddr) 
                );
    return(nbytes);
}

////////////////////////////////////////////////////////////////////////////
void UdpClnDelete( UdpCln *ucl )
{
    close( ucl->sockFd );
    free( ucl );
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#ifdef _UNIT_TEST_

// g++ -D_UNIT_TEST_ net.c -o nettest

int main( int argc, char *argv[] )
{
   int        err;
   int        nb;
   TcpClnCon *tcc;
   char      cmdStr[4096];

   int         svcPort  = 6000;
   const char *svcIpStr = "127.0.0.1";

   svcIpStr = argv[1]; // "192.168.0.2";
   svcPort  = 6790;

   tcc = TcpClnConNew();
   err = TcpClnConnect( tcc , svcIpStr, svcPort, 100 );
   if( err ){
       printf("connect failure %d\n",err);
       return( -1 );
   }

   sprintf(cmdStr,"help\n");
   nb=TcpClnSend( tcc, (unsigned char*)cmdStr, strlen(cmdStr) );
   printf("snd=%d:%s",nb,cmdStr);

   nb=TcpClnRecv( tcc, (unsigned char*)cmdStr, sizeof(cmdStr) );
   printf("rcv=%d:%s",nb,cmdStr);

   return( 0 );
}

#endif
