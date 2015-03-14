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
package Models;

import Devs.DevUtil;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.util.StringTokenizer;

/**
 * This class implements a signal generator engine for a BBB brec module set.
 * 
 * @author user
 */
public class SgEngine extends Thread {

    /** Control socket buffered reader */
    private BufferedReader mCtlBr;
    
    /** Control socket buffered writer */
    private PrintWriter    mCtlPw;
    
    /** Control socket */
    private Socket         mCtlSock;

    /** Device address in ./: notation */
    private String         mIpAddrPortStr;
    
    /** Device run indicator */
    private boolean        mbRun=false;
        
    /** Short human readable status/error string */
    private String         mErrStr;
    
    /** Requested new frequency */
    private long           mNewFreqHz  = 0;
    
    /** Requested new attenuation level */
    private long           mNewAttenDb = 0;
    
    /** Requested new Aux0 */
    private int            mNewAux0Enable = 0;
    
    /** Requested new Aux1 */
    private int            mNewAux1Enable = 0;
    
    /** Requested power state */
    private int            mNewPowerEnable = 1;
    
    /** Requested mode */
    private int            mNewMode = 0;
        
    /** Attenuation last set */
    private long           mAttenDb = -1;
    
    /** Frequency last set */
    private long           mFreqHz  = -1;
    
    /** Aux0 enable last set */
    private int            mAux0Enable;
    
    /** Aux1 enable last set */
    private int            mAux1Enable;
    
    /** Power enable last set */
    private int            mPowerEnable;
    
    /** Mode last set */
    private int            mMode;

    /**
     * This is the constructor.  It does not access the device.
     */
    public SgEngine(){
        mErrStr = "RDY"; // mark current status as ready 
    }

    /**
     * Return current error/status string
     * @return 
     */
    public  String getErrStr(){
        return(mErrStr);
    }
         
    /** 
     * Enable/Disable engine from running.  When running will access device.
     * @param run 
     */
    public void SetRun( boolean run ){
            mbRun = run;
    }
    public boolean GetRun(){
            return(mbRun);
    }
    
    /**
     * Configure the device string (.: notation)
     * @param ipAddrPortStr 
     */
    public void SetInputStr( String ipAddrPortStr ){
        mIpAddrPortStr = ipAddrPortStr;
    }
    public String GetInputStr( ){
        return(mIpAddrPortStr);
    }

    /**
     * Request setting new output frequency.
     * @param fHz 
     */
    public void SetFreq( long fHz ){
        System.out.println("newHz = "+fHz);
        mNewFreqHz = fHz;          
    }

    /**
     * Request setting new attenuation factor
     */
    public void SetAtten( long dB ){
        System.out.println("newAtten = "+dB);
        mNewAttenDb = dB;        
    }
    
    public void SetAux0( int enable ){
        mNewAux0Enable = enable;   
    }
    
    public void SetAux1( int enable ){
        mNewAux1Enable = enable; 
    }

    public void SetPower( int enable ){
        mNewPowerEnable = enable; 
    }
    
    public void SetMode( int mode ){
        mNewMode = mode;
    }
        
    /**
     * Last reported LO 0 frequency
     */
    long mLo0Hz = 1;
    public long getLo0Hz(){ return(mLo0Hz); }
    
    /**
     * Last reported LO 1 frequency 
     */
    long mLo1Hz = 1;
    public long getLo1Hz(){ return(mLo1Hz); }
    
    /**
     * This is part of the main processing loop.
     * This method is periodically called to check and reconfigure the
     * device if necessary.  We use this approach so client threads
     * do not collide with internal thread processing and monitoring when
     * interfacing to the device.
     */
    private void Reconfigure()
    {
        System.out.println("Reconfigure Enter ");
        
        if( null==mCtlPw ) return;
        
        try{
            String  results;
            boolean getLo = false;
             
            if( mNewMode != mMode ){
                mMode = mNewMode;
                System.out.println("SgEngine:Mode = "+mMode);
                mCtlPw.println("sg-mode " + mMode +"\n" ); 
                getLo = true;
            }
                        
            if( mNewAttenDb != mAttenDb ){
                mAttenDb = mNewAttenDb;
                System.out.println("SgEngine:SetAtten = "+mAttenDb);
                mCtlPw.println("sg-adb " + mAttenDb +"\n");  
                getLo = true;
            } 
            
            if( mNewFreqHz != mFreqHz ){
                mFreqHz = mNewFreqHz;
                System.out.println("SgEngine:SetFreq = "+mFreqHz);
                mCtlPw.println("sg-hz " + mFreqHz +"\n" ); 
                getLo = true;
            }
            
            if( mNewAux0Enable != mAux0Enable ){
                mAux0Enable = mNewAux0Enable;
                mCtlPw.println("sg-aux-enable 0 "+mAux0Enable);
            }
            
            if( mNewAux1Enable != mAux1Enable ){
                mAux1Enable = mNewAux1Enable;
                mCtlPw.println("sg-aux-enable 1 "+mAux0Enable);
            }
            
            if( mNewPowerEnable != mPowerEnable ){
                mPowerEnable = mNewPowerEnable;
                mCtlPw.println("sg-power-enable 0 "+mPowerEnable);
                mCtlPw.println("sg-power-enable 1 "+mPowerEnable);
            }
                        
            if( getLo ){ 
                mCtlPw.println("sg-get-lo-hz 0\n");
                mCtlPw.flush();
                results = mCtlBr.readLine();
                mLo0Hz = Long.valueOf(results);                

                mCtlPw.println("sg-get-lo-hz 1\n");
                results = mCtlBr.readLine();
                mLo1Hz = Long.valueOf(results);
            }   
    
            
        }catch( Exception e ){
            System.out.println("SgEngine:Reconfigure e="+e);
            mbRun = false;
        }
    }
        
    ///////////////////////////////////////////////////////////////////////////
    private void OpenLine(  String ipAddrPortStr )
    {            
        // e.g. ipAddrPortStr = "192.168.16.131:8353";
        StringTokenizer strTok = new StringTokenizer(ipAddrPortStr,":");
        
        String ipStr = strTok.nextToken();
        int    ipPort= Integer.parseInt( strTok.nextToken() ); 
        
        if( null==ipStr ) ipStr ="127.0.0.1";
        if( 0   ==ipPort) ipPort=8000;
        
        mErrStr = "OPN";
        
        // Setup the control connection
        try{            
	    SocketAddress sockaddr = new InetSocketAddress(
                                          InetAddress.getByName(ipStr), ipPort);
 
	    mCtlSock = new Socket();
	    mCtlSock.connect(sockaddr, 500 /* timeoutMs */);

            mCtlBr   = new BufferedReader(
                             new InputStreamReader(mCtlSock.getInputStream()));
            mCtlPw   = new PrintWriter(mCtlSock.getOutputStream(), true);
        }
        catch( Exception e ){
            mErrStr = "SER";
        }

    }
    
    ///////////////////////////////////////////////////////////////////////////
    private void CloseLine(){
        try{
            mCtlSock.close();
        }
        catch( Exception e ){
            System.out.println("socket close "+e);
        }
        mCtlPw = null;
    }
    
    public void Exit(){
        mbRun        = false;
        mbThreadExit = true;
    }
    
    public boolean IsExited(){
        return( mbThreadIsExited );
    }
    
    ///////////////////////////////////////////////////////////////////////////
    boolean mbThreadExit     = false;
    boolean mbThreadIsExited = false;
    public void run() {
        System.out.println("SgEngine thread starting");
        while(mbThreadExit==false){
            
            while( !mbRun && !mbThreadExit ){
                try{
                    Thread.sleep(100);
                }catch( Exception e ){
                        ;
                }
            }
            mErrStr = "RUN";
            OpenLine( mIpAddrPortStr);
            while ( mbRun ) {
                try{
                    Thread.sleep( 100 );
                }catch( Exception e ){
                    System.out.println("sleep e="+e);
                }
                Reconfigure();
            }
            CloseLine();
                
        }// End of loop over mbThreadExit
        
        mbThreadIsExited = true;
        System.out.println("SgEngine thread exiting");
    } // End of run

}