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

#ifndef __NETH__
#define __NETH__

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <fcntl.h>      /* for fcntl() for non blocking operations */
#include <errno.h>      

/**
 * This is a multicast group client object to be used for listening for
 * and receiving multicast traffic.
 */
typedef struct {
    int            sockFd;
    struct ip_mreq grpAddr;
} McgCln;

McgCln *McgClnNew( char *mcIpStr, int mcIpPort );
int     McgClnRead( McgCln *mcc, char *bf, int nbytes );
void    McgClnDelete( McgCln *mcc );

/**
 * This is a multicast group server object to bused for sending multicast
 * traffice.
 */
typedef struct {
    int                sockFd;
    struct sockaddr_in grpAddr;
} McgSvr;

McgSvr    *McgSvrNew( const char *mcIpStr, int mcIpPort );
int        McgSvrSend( McgSvr *mcs, unsigned char *sndBf, int sndBytes );
void       McgSvrDelete( McgSvr *mcs );

/**
 * This is a tcp client connection object used to connect to and communicate
 * with tpc connection servers.
 */
typedef struct {
    int        sockFd;
} TcpClnCon;
TcpClnCon *TcpClnConNew();
void       TcpClnConDelete( TcpClnCon *tcc );
int TcpClnConnect(
      TcpClnCon  *tcc,
      const char *svcIpStr,
      int         svcPort,
      int         msTimeout );
int TcpClnSend( TcpClnCon *tcc, unsigned char *sndBf, int sndBytes );
int TcpClnRecv( TcpClnCon *tcc, unsigned char *rcvBf, int rcvBytes );




/**
 * This is a tcp server connection object used to create service points,
 * listent for and service tcp connections.
 */
typedef struct {
    int                 sockFd;
    struct sockaddr_in  peerAddr;
} TcpSvrCon;

TcpSvrCon *TcpSvrConNew();
int        TcpSvrConBind( TcpSvrCon *tsc, int svcPort );
int        TcpSvrConAccept( TcpSvrCon *tsc );
int        TcpSvrConWaitNewCon( TcpSvrCon *tsc, int svcPort );
int        TcpSvrGetClnIpStr( TcpSvrCon *tsc, char *str, int strLen );
int        TcpSvrGetSvcIpStr( TcpSvrCon *tsc, char *strOut, int strLen );
int        TcpSvrRead( TcpSvrCon *tsc, char *bf, int nbytes );
int        TcpSvrReadReady( TcpSvrCon *tsc );
int        TcpSvrWrite( TcpSvrCon *tsc, const char *bf, int nbytes );
void       TcpSvrClose( TcpSvrCon *tsc );

/**
 * This object is a udp server object.
 */
typedef struct {
    int                sockFd;
    struct sockaddr_in lastRcvAddr;
} UdpSvr;
UdpSvr    *UdpSvrNew( int svcPort, int rcvBcast );
int        UdpSvrSendto( UdpSvr *usv, 
                         const char *dstIpStr, int dstPort, 
                         unsigned char *sndBf, int sndBytes );
int        UdpSvrRecv( UdpSvr *usv, unsigned char *rcvBf, int rcvBytes );
void       UdpSvrDelete( UdpSvr *usv );

/**
 * This object is a udp client object.
 */
typedef struct {
    int                sockFd;
    struct sockaddr_in dstAddr;
} UdpCln;
UdpCln    *UdpClnNew();
void       UdpClnSetDst( UdpCln *ucl, char *dstIpStr, int dstPort );
int        UdpClnSend (UdpCln *ucl, unsigned char *sndBf, int sndBytes );
void       UdpClnDelete( UdpCln *ucl );

#endif
