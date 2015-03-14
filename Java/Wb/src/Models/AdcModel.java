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

import java.io.Serializable;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;

/**
 * This is the network ADC model.  It provides a time series and spectral
 * estimate of ADC data.  It provides a set of bean interfaces usable with
 * instrumentation widgets to control the device and processing.
 */
public class AdcModel implements ModelInterface, Serializable {

    static final long                       serialVersionUID = 42L;
    private transient AdcEngine             mAdcEngine;
    private transient PropertyChangeSupport mPcs;
    
    public void addPropertyChangeListener(  PropertyChangeListener listener ){
        mPcs.addPropertyChangeListener(listener);
    }
    
    public void removePropertyChangeListener(  PropertyChangeListener listener ){
        mPcs.removePropertyChangeListener(listener);
    }    
    
    public AdcModel(){
        mAdcEngine = new AdcEngine(); 
        mPcs       = new PropertyChangeSupport( this );
        
        setStateStr(     "OFF");
        setStatusStr(    "---");
        setFftSizeStr(   "128");
        setAveSizeStr(   "1");  
        
        //setDevStr(       "192.168.0.2:6787:udp");
        //setDevStr(       "192.168.0.2:6788:tcp");
        setDevStr( "192.168.0.2:6788:tcp");
    }
   
    // Xy data output stream version indicator
    public int GetNewXyVer( int ver ){
        return( mAdcEngine.GetNewXyVer(ver) );
    }
    
    // Xy data output retrieval
    public int GetNewXyData( int chnl, double [] x, double [] y, int maxPoints ){
        int npts;
        npts = mAdcEngine.GetNewXyData(chnl, x,y,maxPoints);
        mReportRbwHz = (x[ npts-1 ] - x[0] ) / npts;
        return( npts  );
    }
    
    // TODO need to figure out how to refresh new values
    public void FireAll(){
        mPcs.firePropertyChange("StateStr",    "",getStateStr());
        mPcs.firePropertyChange("StatusStr",   "",getStatusStr());
        mPcs.firePropertyChange("FftSizeStr",  "",getFftSizeStr());
        mPcs.firePropertyChange("AveSizeStr",  "",getAveSizeStr());
        mPcs.firePropertyChange("DevStr",      "",getDevStr());
    }
        
    /**
     * mState - State of instrument.  ON or OFF.
     */
    String  mStateStr;
    boolean mStateOn;
    public void setStateStr( String newStr ){
        String prevStr = mStateStr;
        mStateStr     = newStr;
        mPcs.firePropertyChange("StateStr",prevStr,newStr);
        
        if( 0==mStateStr.compareTo("ON") ){
            mStateOn = true;
            mAdcEngine.SetRun(true);
        }
        else{
            mStateOn = false;
            mAdcEngine.SetRun(false);
        }
    }
    public String getStateStr(){
        return(mStateStr);
    }
    
    /**
     * mStatusStr - Status of instrument. ---, OK, LI, ...
     */
    String mStatusStr;
    public void setStatusStr( String newStr ){
        String prevStr = mStatusStr;
        mStatusStr     = newStr;
        mPcs.firePropertyChange("StatusStr",prevStr,newStr);
    }
    public String getStatusStr(){
        return(mStatusStr);
    }
        
    /**
     * mFftSize - Number of points in FFT. Numeric value, power of 2.
     */
    String mFftSizeStr;
    double mFftSize;
    public void setFftSizeStr( String newStr ){
        String prevStr = mFftSizeStr;
        mFftSize    = Double.parseDouble(newStr);
        mFftSizeStr = newStr;
        mPcs.firePropertyChange("FftSizeStr",prevStr,newStr);
        mAdcEngine.Configure( (int)mAveSize, (int)mFftSize);
    }
    public String getFftSizeStr(){
        return( mFftSizeStr );
    }
    
    /**
     * mAveSize - Number of averages to conduct. Numeric value.
     */
    String mAveSizeStr;
    double mAveSize;
    public void setAveSizeStr( String newStr ){
        String prevStr = mAveSizeStr;
        mAveSize    = Double.parseDouble(newStr);
        mAveSizeStr = newStr;
        mPcs.firePropertyChange("AveSizeStr",prevStr,newStr);
        mAdcEngine.Configure( (int)mAveSize, (int)mFftSize);
    }
    public String getAveSizeStr(){
        return( mAveSizeStr );
    }
    
    /**
     * mInputStr - Network device address and port (e.g. 192.168.0.2:8353)
     */
    public void setDevStr( String newStr ){
        mPcs.firePropertyChange("DevStr",mAdcEngine.GetInputStr(),newStr);
        mAdcEngine.SetInputStr( newStr );    
    }
    public String getDevStr(){
        return( mAdcEngine.GetInputStr() );
    }
    
    public void Exit(){
        mAdcEngine.Exit();
    }
            
    public void Start(){
        mAdcEngine.start();
    }
    
    public void UiUpdateRequest(){
        if( mStateOn ){
            setStatusStr( mAdcEngine.getErrStr() );
        }
        else{
            setStatusStr( "---" );
        }
    }

    private double mReportRbwHz = 1.0;
    public String GetReportStr1(){
        return("ADC:FFT="+mAdcEngine.GetFftPoints()+",AVE="+mAdcEngine.GetAveCount() );
    }
    public String GetReportStr2(){
        return("ADC:RBW(Hz)="+(int)(mReportRbwHz)+",Fs="+mAdcEngine.GetFs());
    }
    
    /** Required model interface */
    public double GetXmax(){
        return(mAdcEngine.GetFs()/2);
    }
    
    /** Required model interface */
    public double GetXmin(){
        return( 0 ); 
    }
}
