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
public class AdcTcp implements AdcNet {
                  
    
    ///////////////////////////////////////////////////
    /**
     * Port address of TCP control port and UDP rcv port
     */
    private int            mIpPort; 
    
    /**
     * BufferedReader for parsing TCP control socket return results
     */
    private BufferedReader mAdcBr;
    
    /**
     * PrintWriter for issuing commands on TCP control socket
     */
    private PrintWriter    mAdcPw;
    
    /**
     * TCP control socket itself
     */
    private Socket         mAdcSock;
    
    /**
     * Error status string - reflects best estimate of most recent error
     */
    private String         mErrStr;
    
    /**
     * Real samples per second of sampling rate.
     */
    private double         mSamplesPerSecond;
  
    /*
     * Constructor takes no actions, only initializes internal defaults
     */
    public AdcTcp(){
        mErrStr           = "OK";
        mSamplesPerSecond = 1000000;
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
               
        // Setup the control connection
        mIpPort = port;
        try{            
	    SocketAddress sockaddr = new InetSocketAddress(
                                          InetAddress.getByName(IpStr), 
                                          mIpPort);
 
	    mAdcSock = new Socket();
	    mAdcSock.connect(sockaddr, timeoutMs);

            mAdcBr   = new BufferedReader(
                             new InputStreamReader(mAdcSock.getInputStream()));
            mAdcPw   = new PrintWriter(mAdcSock.getOutputStream(), true);
        }
        catch( Exception e ){
            mErrStr="SKE";
            DevUtil.LogMsg("socket:"+e);
            return(-1);
        }

        return(0);
    }
        
    /**
     * This method closes the device by signaling the UDP receive thread
     * to exit and closing the control connection.
     * @return 
     */
    public int Close(){
        
        // Clean up the control socket
        try{
            mAdcSock.close();
        }
        catch( Exception e ){
            DevUtil.LogMsg("socket"+e);
            return(-1);
        }
        
        // Done either way
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
     
    int GetSockUshort() throws Exception
    {
        int msb, lsb;
        try{
            msb = mAdcSock.getInputStream().read();
            lsb = mAdcSock.getInputStream().read();
            return( (lsb<<8) + msb );
        }
        
        catch( Exception e){
            throw( e );
            // return( -1 );    
        }
    }
    
    /**
     * This method retrieves the last un-consumed number of samples from 
     * the UDP receiver.
     * @param samples
     * @param nSamples
     * @return 0 on success reading nSamples, <0 else
     */
    public int GetStreamSamples( double samples[], int nSamples ){
        int didx;
        int synch,nSamps,spsMsb,spsLsb,seqNo,spare;
        int cnt;
          
        spsMsb = 0;
        spsLsb = 100;
        
        // Initiate a coherent sample set transfer
        mAdcPw.println("s");      
       
        // Parse the results into samle buffer
        try{

            didx = 0;
            while( didx < nSamples ){
                
                synch = GetSockUshort();
                if( synch!=0xa5a5 ){
                    DevUtil.LogMsg("sequencing error "+synch);
                    Thread.sleep( 1 );
                    mAdcSock.getInputStream().skip( mAdcSock.getInputStream().available() );
                    return( -1 );
                }
                
                nSamps = GetSockUshort();
                spsMsb = GetSockUshort();
                spsLsb = GetSockUshort();
                seqNo  = GetSockUshort();
                spare  = GetSockUshort();
                
                // DevUtil.LogMsg("parsing shorts =  "+nSamps);
                cnt = 0;
                while( (cnt < nSamps) & (didx<nSamples) ){
                    samples[ didx ] = GetSockUshort();
                    cnt++;
                    didx++;
                }
                

            }
            mSamplesPerSecond = (spsMsb<<16) + spsLsb;
            DevUtil.LogMsg("sps="+mSamplesPerSecond);
            return( 0 );
        }
        catch( Exception e ){
            DevUtil.LogMsg("GetStreamSamples exception "+e);
            return( -1 );
        }
    }
}