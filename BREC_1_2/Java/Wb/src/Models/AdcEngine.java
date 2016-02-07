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

import Devs.AdcNet;
import Devs.AdcUdp;
import Devs.AdcTcp;
import java.util.StringTokenizer;

/**
 * This class is the engine for a ADC network interface.  It produces
 * two XY outputs: a) a raw time series from the device, and b) a spectral
 * estimate of the device output.
 */
public class AdcEngine extends Thread {

    /** UDP device used to collect samples from the network */
    private AdcNet         mDevAdc;

    /** Samples per second - updated by network device open */
    private double         mSamplesPerSecond = 44100;
    
    /** Maximum number of FFT points to work with 
      * NOTE: This is also the number of coherent points assumed
      */
    private int            mMaxFftPoints     = 32768;
    
    /** Maximum number of averages to work with */
    private int            mMaxAveCount      = 32;
    
    /** Flag indicating if a full +/- set of frequencies should be used */
    private boolean        mReportFull       = false;

    /** Working variable for current averaging count in effect */
    private int            mAveCount;
    
    /** Working variable for current number of FFT points in effect */
    private int            mFftPoints;
    
    /** Working FFT parameters and structure */
    private FFT            mFft;

    /** Working samples directly from ADC device (no windowing) */
    private double[]       chnlA;
    private double[]       chnlB;

    /** Average of mag squared data and number of averages within the data*/
    private double[]       mAveM2data;
    private int            mCurAveCount;

    /** Windowed real and imaginary data */
    private double[]       mWinRdata;
    private double[]       mWinIdata;
    
    /** Final output data - x in Hertz, y in dBFS16 */
    private double[]       mXdataOut;
    private double[]       mYdataOut;
    
    /** Final output data - x in sample numbers, y in device ADC values
     * NOTE: The time series data is snapped from the input only when 
     * a frequency update is ready. i.e. multiple time series will be consumed
     * to produce a frequency output update (based on fft size, averaging), 
     * only the last ADC sample set is used for this update.
     */
    private double[]       mY2dataOut;
    private double[]       mX2dataOut;
    
    /** Running counter of output data versions/updates */
    private int     mDataVer; 
    
    /** New set of fft points to use in the next configuration cycle
     * See Configure/Reconfigure.
     */
    private int mNewFftPoints;
    
    /** New set of average count to use in the next configuration cycle 
     * See Configure/Reconfigure
     */
    private int mNewAveCount;

    /** Short, human readable string reflecting last error encountered */
    private String         mErrStr;
    
    /** Internal flag indicating if engine should be running/processing */
    private boolean mbRun=false;
                
    /** String reflecting device IP address and port in ./: notation    */
    private String mIpAddrPortStr;
    
    /** Flag to primary thread to initiate exit/termination */
    private boolean mbThreadExit     = false;
    
    /** Indicator set only when primary thread has terminated */
    private boolean mbThreadIsExited = false;
        
    /**
     * This method creates an engine object.  It creates a thread to 
     * interface to the device.  The device is not opened until the
     * connection parameters are set and the SetRun method is invoked.
     */
    public AdcEngine(){
       
        chnlA   = new double[mMaxFftPoints];
        chnlB   = new double[mMaxFftPoints]; 
        for(int idx=0;idx<(mMaxFftPoints);idx++){
            chnlA[idx] = 0.0;
            chnlB[idx] = 0.0;
        }

        mAveM2data= new double[mMaxFftPoints];

        mWinRdata = new double[mMaxFftPoints];
        mWinIdata = new double[mMaxFftPoints];

        mXdataOut = new double[mMaxFftPoints];
        mYdataOut = new double[mMaxFftPoints];

        mX2dataOut= new double[mMaxFftPoints];
        mY2dataOut= new double[mMaxFftPoints];
        
        mFftPoints    = 64;
        mAveCount     = 16;
        mNewFftPoints = 1024;
        mNewAveCount  = 1;
        
        // Need an fft object to start with 
        // until configure/reconfigure gets invoked
        mFft       = new FFT(mFftPoints);
        
        // Mark current status string as ready
        mErrStr = "RDY";
    }

    /** 
     * Set the network address of the ADC device. Format of address is
     * ./: notation (i.e. 192.168.0.2:6787)
     * @param ipAddrPortStr 
     */
    public void SetInputStr( String ipAddrPortStr ){
        mIpAddrPortStr = ipAddrPortStr;
    }
    
    /**
     * Produces a string representing the device address in ./: notation.
     * @return 
     */
    public String GetInputStr( ){
        return(mIpAddrPortStr);
    }
    
    /**
     * Returns short human readable string describing current state of engine.
     * @return 
     */
    public String getErrStr(){
        return(mErrStr);
    }
        
    /**
     * Returns the maximum number of FFT points that can be used.
     * @return 
     */
    public int GetMaxFftPoints(){
        return( mMaxFftPoints );   
    }
      
    /**
     * Returns the samples per second in use.
     * @return 
     */
    public double GetFs()
    {
        return( mSamplesPerSecond );
    }
    
    /**
     * Returns the average count in use.
     * @return 
     */
    public int GetAveCount(){
        return( mAveCount );
    }
    
    /**
     * Returns the number of fft points in use.
     * @return 
     */
    public int GetFftPoints(){
        return(mFftPoints);
    }

    /**
     * Sets the engine run state to value provided.
     * NOTE: Running the device will initiate device network contact, hence,
     * the network parameters should be valid prior to enabling run.
     * @param run 
     */
    public void SetRun( boolean run ){
        mbRun = run;
    }

    /**
     * Returns the current run state of the engine.
     * @return 
     */
    public boolean GetRun(){
        return(mbRun);
    }
    
    /**
     * This method returns the current version of available XY data.
     * @param ver
     * @return 
     */
    public int GetNewXyVer( int ver ){
        return( mDataVer );
    }
    
    /**
     * This method copies the current XY data to the location provided.
     * The maximum of the provided maxPoints or internal maximum points
     * available is copied. It returns the number of points produced.
     * 
     * NOTE: channel (0=spectrum, 1=time series).
     * 
     * @param chnl
     * @param x
     * @param y
     * @param maxPoints
     * @return 
     */
    public int GetNewXyData( int chnl, double [] x, double [] y, int maxPoints ){
        int idx,sidx,didx;
        int max;

        if( 0==chnl ){
            if( mReportFull ){
                max = Math.min(maxPoints, mFftPoints);
                for(idx=0;idx<max;idx++){
                    x[idx] = mXdataOut[idx];
                    y[idx] = mYdataOut[idx];
                }
            }
            else{
                max = Math.min(maxPoints, mFftPoints/2);
                for(didx=0,sidx=mFftPoints/2;didx<max;sidx++,didx++){
                    x[didx] = mXdataOut[sidx];
                    y[didx] = mYdataOut[sidx];
                }
            }
            return( max ); 
        }
        else{
            max = Math.min(maxPoints, mFftPoints);      
            for(idx=0;idx<max;idx++){
                x[idx] = mX2dataOut[idx];
                y[idx] = mY2dataOut[idx];
            }
            return(max);
        }
    }

    
    /**
     * This method is the primary external thread safe method used to
     * set new parameters for the engine.  It saves the new parameters
     * until the engine is in an acceptable point within the processing
     * cycle to apply and use the new parameters.
     * @param reqAvCount
     * @param reqFftPoints 
     */
    public void Configure( int reqAvCount, int reqFftPoints ){

        if( reqFftPoints > mMaxFftPoints ){
            mNewFftPoints = mMaxFftPoints;
        }
        else{
            mNewFftPoints = reqFftPoints;
        }
        if( reqAvCount > mMaxAveCount ){
            mNewAveCount = mMaxAveCount;
        }
        else{
            mNewAveCount = reqAvCount;
        }
    }
    
    /**
     * Processing loop primitive: updates processing parameters
     * This method is invoked each computation cycle to check for and
     * apply if necessary a new set of computation parameters.
     */
    private void Reconfigure(){
        if( mFftPoints!= mNewFftPoints       ){
            mFftPoints = mNewFftPoints;
            mFft       = new FFT(mFftPoints);
        }
        if( mAveCount!=mNewAveCount ){
            mAveCount = mNewAveCount;
        }
    }
    
    /**
     * Processing loop primitive: collects max fft samples from the device
     * placing the results in chnlA.
     */
    private void CollectSamples(){
        int err;
        err = mDevAdc.GetStreamSamples(chnlA, mMaxFftPoints);
        if( 0!=err ){
            mErrStr = "RDE";
        }   
    }

    /**
     * Processing loop primitive: Process the samples in chnl A/B updating
     * the running mag squared average.
     * 
     * NOTE: Averaging is done on mag2.  Averaging on Re/Im reduces peak (and
     * noise) due to moving phase.  Averaging on mag2 reduces noise average
     * while retaining changing phase non-noise signals.
     */
    private void ProcessSamples(){
		
    	int dstIdx;
    	int srcIdx;
    	double [] window;
    	
    	window = mFft.getWindow();

        // Copy samples to fft workspace
        dstIdx = 0;
        srcIdx = 0;
        while( dstIdx<mFftPoints ){
            mWinRdata[dstIdx] = window[dstIdx] * chnlA[srcIdx];
            mWinIdata[dstIdx] = window[dstIdx] * chnlB[srcIdx];
            srcIdx++;
            dstIdx++;
        }

        // Perform the fft 
        mFft.fft(mWinRdata, mWinIdata);

        // Place results in average
        dstIdx=0;
        while( dstIdx<mFftPoints ){
            mAveM2data[dstIdx] += ( 
                                    (mWinRdata[dstIdx]*mWinRdata[dstIdx]) +
                                    (mWinIdata[dstIdx]*mWinIdata[dstIdx])
                                   );
            dstIdx++;
        }   
        
        // Update the current average count
        mCurAveCount++;
    }
    /**
     * Processing loop primitive: This method will take the completed 
     * averaged M2 data normalize it, shift it around, create an X set of 
     * values, and store away as new data.  Included is the snapshot of a 
     * time series.
     * 
     * The normalization and output levels proceed as follows:
     * a) A DC time series of half scale will produce an output of -6dB
     * b) A sin time series of full scale, is a DC center (DC half scale value)
     *    with a sinusoid of half scale +/-.  So if N/2 is the DC balance, N/2 is
     *    the sinusoid amplitude, the peak value is N = N/2 + N/2 * 1.0 and
     *    the min value is 0 = N/2 + N/2 * -1.0.  This sinusoid is really split
     *    in power between the positive and negative e(jw)'s.  In addition the
     *    power in a sinusoid is rms.  Said differently, RMS=(0.707*N/2 )*0.5
     *    which puts us 20*log10( 0.17675 = 0.707*0.5*0.5 ) = -7.5dB
     * c) The windowing function also has an effect.
     * TODO: The above does not include the window amplitude correction factor
     */
    private void FormatSamples(){
        double x,y,w;
        int    srcIdx,dstIdx;

        mSamplesPerSecond = mDevAdc.GetSamplesPerSecond();
        
    	// Normalize - divide by average count, divide by fft points squared
        //  and divide by (2pi*2^16)^2, then mulitplied by the window amplitude
        //  correction factor squared.
        // NOTE: The computation is more efficient to construct a log 
        // subtraction one time for the division rather than doing it within
        // the loop.
        // NOTE: Beyond efficiency considerations the real reason to conduct
        // the computation this way is to avoid floating point error issues.
        // In the cases of large fft sizes and large averages the normalizing
        // factor becomes  problematic.
        double r= (2<<16)/ (2.0*Math.PI);
        double dBref = r*r;
        double norm = 10*Math.log10( dBref ) + 
                      10*Math.log10( mFftPoints ) +
                      10*Math.log10( mFftPoints ) +
                      10*Math.log10( mAveCount )  - 
                      20*Math.log10( mFft.getWindowAcf() );
        
        // Convert to DB and place in output
        // NOTE: This is with shift around, two conjoined loops
        srcIdx=0;
        dstIdx=(mFftPoints/2);
        while( dstIdx<mFftPoints ){
            w = 10* Math.log10( mAveM2data[srcIdx] );
            y = w - norm; 
            x = (srcIdx*mSamplesPerSecond/mFftPoints);
            mXdataOut[dstIdx] = x;
            mYdataOut[dstIdx] = y;
            srcIdx++;
            dstIdx++;
        }
        dstIdx = 0;
        while( srcIdx<mFftPoints ){
            w = 10* Math.log10( mAveM2data[srcIdx] );
            y = w - norm; 
            x = -mSamplesPerSecond + (srcIdx*mSamplesPerSecond/mFftPoints);
            mXdataOut[dstIdx] = x;
            mYdataOut[dstIdx] = y;
            srcIdx++;
            dstIdx++;
        }
        // System.out.println("sps="+mSamplesPerSecond);
        
        // Zero the average for next time 
    	dstIdx=0;
    	while( dstIdx<mFftPoints ){
    		mAveM2data[dstIdx] = 0;
    		dstIdx++;
    	}
        
        // Reset the current average count
        mCurAveCount = 0;
        
        // Snap a copy of parital time series
    	dstIdx=0;
    	while( dstIdx<mFftPoints ){
    		mX2dataOut[dstIdx] = dstIdx;
                mY2dataOut[dstIdx] = chnlA[dstIdx];
    		dstIdx++;
    	}
        
        // Indicate that new data is ready
        mDataVer++;
    }
            
    /**
     * Processing loop primitive: This method is invoked to open the device
     * before any processing loop activities are conducted.  The address
     * of the device must be configured properly prior to this.
     * 
     * @param ipAddrPortStr 
     */
    private void OpenLine(  String ipAddrPortStr )
    {       
        int err;
       
        // e.g. ipAddrPortStr = "192.168.16.131:8353";
        //      ipAddrPortStr = "192.168.0.2:6788:udp";
        //      ipAddrPortStr = "192.168.0.2:6788:tcp";
        StringTokenizer strTok = new StringTokenizer(ipAddrPortStr,":");
        
        String ipStr = strTok.nextToken();
        int    ipPort= Integer.parseInt( strTok.nextToken() ); 
        
        String protStr = strTok.nextToken();
        
        if( null==ipStr )   ipStr ="127.0.0.1";
        if( 0   ==ipPort)   ipPort=8000;
        if( null==protStr ) protStr="tcp";
        
        mErrStr = "OPN";
        
        if( 0==protStr.compareTo("udp") ){
            System.out.println("Opening udp interface");
            mDevAdc = new AdcUdp();
        }
        else{
            System.out.println("Opening tcp interface");
            mDevAdc = new AdcTcp();  
        }
       
        
        err = mDevAdc.Open(ipStr, ipPort, 1000); // 1000mS timeout
        if( 0!=err ){
          mErrStr = "OER";    
        }
        
    }
    
    /**
     * Processing loop primitive: This method is invoked after all loop 
     * processing is done to close the ADC network device.
     */
    private void CloseLine(){
        if( null==mDevAdc) return;
        mDevAdc.Close();
        
    }
    
    /**
     * This method signals an instance of this object to stop running and
     * all associated threads to exit.  NOTE: the threads exit async and
     * are not complete at the return of this method.
     */
    public void Exit(){
        mbRun        = false;
        mbThreadExit = true;
    }
    
    /**
     * This method returns an indication if the associated thread(s) with this
     * object have completed their termination.
     * @return 
     */
    public boolean IsExited(){
        return( mbThreadIsExited );
    }

    /**
     * This is the main thread entry point and part of the task interface, it
     * is not to be invoked directly.  It will be invoked when an object of 
     * this type is instantiated.
     */
    public void run() {
        System.out.println("AdcEngine thread starting");
        while(mbThreadExit==false){
            
            // Pause if we are not running but not exiting
            while( !mbRun && !mbThreadExit ){
                try{
                    Thread.sleep(100);
                }catch( Exception e ){
                        ;
                }
            }
            
            // Primary processing loop
            mErrStr = "RUN";
            OpenLine( mIpAddrPortStr);
            while ( mbRun ) {
                Reconfigure();
                CollectSamples();
                ProcessSamples();
                if( mCurAveCount >= mAveCount ){
                    FormatSamples();
                }
            }
            CloseLine();
                
        }// End of loop over mbThreadExit
        
        mbThreadIsExited = true;
        System.out.println("AdcEngine thread exiting");
    } // End of run
    
}// End of class