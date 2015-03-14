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
package Devs;

import java.io.BufferedReader;
import java.io.PrintWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.InetSocketAddress;
import java.net.InetAddress;
import java.net.DatagramPacket;
import java.net.DatagramSocket;


/**
 * This class implements a TCP control connection to an ADC receiver and
 * a UDP receiver of 16 bit sample words.
 * 
 * @author user
 */
public class AdcUdp implements AdcNet {
               
    /**
     * This nested class is a dedicated UDP receiver.
     */
    public class UdpRcvThread implements Runnable {
        boolean            mbThreadExit; // Indicator for rcv thread to exit
        DatagramPacket[]   rcvPkts;      // Fifo or pkts for receiving into
        int                rcvPktCount;  // Number of rcvPkts in queue
        int                mHidx;        // head pkt index (take here)
        int                mTidx;        // tail pkt index (rcv here)
        int                mBidx;        // byte pos within current pkt

        /**
         * The constructor establishes 500 receive buffers each of 1600 bytes.
         */
        public UdpRcvThread(){
            int    idx;
            byte[] rcvPktBf;
            
            mbThreadExit = false;
            rcvPktCount  = 500;
            rcvPkts = new DatagramPacket[ rcvPktCount ];
            for(idx=0;idx<rcvPktCount;idx++){
                rcvPktBf = new byte[ 1600 ];
                rcvPkts[ idx ] = new DatagramPacket( rcvPktBf, rcvPktBf.length);    
            }
        }
        
        /**
         * This method extracts the requested number of samples from 
         * the incoming stream.  It will wait for up to 500 mS.  
         * It returns 0 if the samples have been received or non zero on error.
         * 
         * @param samples
         * @param nSamples
         * @return 
         */
        public int GetSamplesUdp( double samples[], int nSamples ){
           double   y;
           int      dIdx; // Output vector destination index
           byte[]   pktSamps;
           int      waitCount;
           int      lsb,msb;
           
           // dIdx/mBidx are dst and src sample indecies respectively
           // mHidx/mTidx are head and tail pkt indecies respectively
           dIdx = 0; 
           while( dIdx < nSamples ){
               waitCount = 0;
               while( mHidx == mTidx ){
                   try { Thread.sleep(50); } catch (Exception e){ ; }  
                   waitCount++;
                   if( waitCount > 10 ) return( -1 );
               }   
               pktSamps = rcvPkts[mHidx].getData();
               msb = pktSamps[ mBidx+1 ] & 0xff;
               lsb = pktSamps[ mBidx   ] & 0xff;
               y =  ( (msb<<8) + lsb );
               samples[ dIdx ] = y;
               dIdx++;
               mBidx+=2;
               if( mBidx >= rcvPkts[mHidx].getLength() ){
                   mBidx = 12; // Start over the 12 byte / 6 sample heaer
                   mHidx = (mHidx+1)%rcvPkts.length;
               }
           }
           return( 0 );
        }
        
        /**
         * This method signals the UDP receive thread to exit.  
         * NOTE: The thread has not necessarily exited at the return of this
         * method.
         */
        public void ExitThread(){
            mbThreadExit = true;    
        }
        
        /**
         * This is the UDP receive thread main entry point.  It sits in a
         * loop receiving UDP messages into the pre-allocated receive packets.
         * When the thread exit indicator is set it closes the socket and
         * exits.
         */
        public void run(){
            DatagramSocket socket = null;
            
            try {
                socket = new DatagramSocket( mIpPort );
                while ( !mbThreadExit ) {
                    socket.receive( rcvPkts[mTidx] );
                    mTidx = (mTidx+1)%rcvPkts.length;
                }
                socket.close();
            } catch (IOException ioe) {
              System.out.println(ioe);
            }
        } // end of run()
    }
    
    
    ///////////////////////////////////////////////////
    /**
     * Port address of TCP control port and UDP rcv port
     */
    private int            mIpPort; 
    
    /**
     * BufferedReader for parsing TCP control socket return results
     */
    private BufferedReader mCtlBr;
    
    /**
     * PrintWriter for issuing commands on TCP control socket
     */
    private PrintWriter    mCtlPw;
    
    /**
     * TCP control socket itself
     */
    private Socket         mCtlSock;
    
    /**
     * Error status string - reflects best estimate of most recent error
     */
    private String         mErrStr;
    
    /**
     * Object of nested class used to receive UDP sample packets
     */
    private UdpRcvThread   mUdpRcvThread;
    
    /**
     * Real samples per second of sampling rate.
     */
    private double         mSamplesPerSecond;
  
    /*
     * Constructor takes no actions, only initializes internal defaults
     */
    public AdcUdp(){
        mErrStr           = "OK";
        mSamplesPerSecond = 1000000;
        mUdpRcvThread     = null;
    }
    
    /**
     * Return a human readable short error string of most recent status
     * @return 
     */
    public String getErrStr(){
        return(mErrStr);
    }
        
    /**
     * This method opens a control connection to the provided address and port
     * It gets the sample rate as a test of the connection (and for data 
     * production).
     * 
     * @param IpStr
     * @param port
     * @param timeoutMs
     * @return 
     */
    public int Open( String IpStr, int port, int timeoutMs ){
        int err;
              
        // Setup the control connection
        mIpPort = port;
        try{            
	    SocketAddress sockaddr = new InetSocketAddress(
                                          InetAddress.getByName(IpStr), 
                                          mIpPort);
 
	    mCtlSock = new Socket();
	    mCtlSock.connect(sockaddr, timeoutMs);

            mCtlBr   = new BufferedReader(
                             new InputStreamReader(mCtlSock.getInputStream()));
            mCtlPw   = new PrintWriter(mCtlSock.getOutputStream(), true);
        }
        catch( Exception e ){
            mErrStr="SKE";
            DevUtil.LogMsg("socket:"+e);
            return(-1);
        }
                
        // Get the samples per second via the command interface
        err = CmdGetSamplesPerSecond();
        if( err!=0 ){
            Close();
            mErrStr="SME";
            return(-1);
        }
        
        // Start a new UDP rcv thread
        mUdpRcvThread = new UdpRcvThread();
        Thread t = new Thread( mUdpRcvThread );
        t.start();
        
        return(0);
    }
        
    /**
     * This method closes the device by signaling the UDP receive thread
     * to exit and closing the control connection.
     * @return 
     */
    public int Close(){
        
        // Clean up the multicast receive thread (if applicable)
        if( mUdpRcvThread!=null ){
            mUdpRcvThread.ExitThread();
            mUdpRcvThread = null;
        }
        
        // Clean up the control socket
        try{
            mCtlSock.close();
        }
        catch( Exception e ){
            DevUtil.LogMsg("socket"+e);
            return(-1);
        }
        
        // Done either way
        return(0);
    }
        
    /**
     * This method issues a TCP control socket command to get the 
     * sample rate.
     * 
     * @return 
     */
    public int CmdGetSamplesPerSecond(){
        try{
            String results;
            mCtlPw.println("adc-samples-per-second");
            results = mCtlBr.readLine();
            DevUtil.LogMsg("results = "+results);
            mSamplesPerSecond =  Integer.valueOf(results);
        }
        catch ( Exception e ){
            DevUtil.LogMsg("sock io: "+e);
            return(-1);
        }
        return(0);
    }

    /**
     * This method returns the number of real samples per second obtained
     * at device open.
     * @return 
     */
    public double GetSamplesPerSecond()
    {
        return( mSamplesPerSecond );    
    }
     
    /**
     * This method retrieves the last un-consumed number of samples from 
     * the UDP receiver.
     * @param samples
     * @param nSamples
     * @return 
     */
    public int GetStreamSamples( double samples[], int nSamples ){
        if( mUdpRcvThread!=null ){
            return( mUdpRcvThread.GetSamplesUdp(samples, nSamples) ); 
        }
        else{
            return(-1);
        }
    }
}