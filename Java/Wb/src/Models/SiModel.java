package Models;

import java.io.Serializable;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;

/**
 *
 * @author jkleine
 */
public class SiModel implements ModelInterface, Serializable {

    static final long serialVersionUID = 42L;

    /**
     * This object is the underlying engine of the model.
     */
    private transient SiEngine              mSaEngine;
    
    /**
     * This is the object used to support property changes
     */
    private transient PropertyChangeSupport mPcs;
    
    public void addPropertyChangeListener(  PropertyChangeListener listener ){
        mPcs.addPropertyChangeListener(listener);
    }
    
    public void removePropertyChangeListener(  PropertyChangeListener listener ){
        mPcs.removePropertyChangeListener(listener);
    }    
    
    public SiModel(){
        mSaEngine = new SiEngine();
        
        mPcs      = new PropertyChangeSupport( this );
        
        setStateStr(      "OFF");
        setStatusStr(     "---");
        setDevStr(        "192.168.0.2:6789");
        //setDevStr(        "192.168.0.81:6789");
        setFcenterMHzStr( "200");
        setSpanMHzStr(    "0.5");  
        setTgPointsStr(   "0" );
        setIfGainStr(     "0");
        setMbwStr(        "500");
        setAveStr(        "1");
        setIntStr(        "OFF");
        setTgPointsStr(   "0");
        setIfGainStr(     "0");
        setMbwStr(        "250");
        setRfChStr(       "0");
        setFftnStr(       "8192");
        setAttencStr(     "0");
        setAttenfStr(     "0");
    }
    
    // Xy data output stream version indicator
    public int GetNewXyVer( int ver ){
        return( mSaEngine.GetNewXyVer(ver) );
    }
    
    // Xy data output retrieval
    public int GetNewXyData( int chnl, double [] x, double [] y, int maxPoints ){
        int npts;
        npts = mSaEngine.GetNewXyData(chnl, x,y,maxPoints);
        mReportRbwHz = 1e6 * (x[ npts-1 ] - x[0] ) / npts;
        return( npts  );
    }
    
    public void FireAll(){
        mPcs.firePropertyChange("StateStr",      "",getStateStr());
        mPcs.firePropertyChange("StatusStr",     "",getStatusStr());
        mPcs.firePropertyChange("FcenterMHzStr", "",getFcenterMHzStr());
        mPcs.firePropertyChange("SpanMHzStr",    "",getSpanMHzStr());
        mPcs.firePropertyChange("DevStr",        "",getDevStr());    
        mPcs.firePropertyChange("TgPointsStr",   "",getTgPointsStr());
        mPcs.firePropertyChange("IfGainStr",     "",getIfGainStr());
        mPcs.firePropertyChange("MbwStr",        "",getMbwStr());
        mPcs.firePropertyChange("RfChStr",       "",getRfChStr() );
        mPcs.firePropertyChange("FftnStr",       "",getFftnStr() );
        mPcs.firePropertyChange("AveStr",        "",getAveStr() );
        mPcs.firePropertyChange("IntStr",        "",getIntStr() );
        mPcs.firePropertyChange("AttencStr",     "",getAttencStr() );
        mPcs.firePropertyChange("AttenfStr",     "",getAttenfStr() );
    }
     
    /**
     * Configure all engine dynamic attributes.
     */
    void ConfigureEngineAll(){
        setFcenterMHzStr( getFcenterMHzStr() );
        setSpanMHzStr( getSpanMHzStr() );
        setTgPointsStr( getTgPointsStr() );
        setIfGainStr( getIfGainStr() );
        setMbwStr( getMbwStr() );
        setRfChStr( getRfChStr() );
        setFftnStr( getFftnStr() );
        setAveStr( getAveStr() );
        setAttencStr( getAttencStr() );
        setAttenfStr( getAttenfStr() );
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
            mSaEngine.SetRun(true);
            ConfigureEngineAll();
        }
        else{
            mStateOn = false;
            mSaEngine.SetRun(false);
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

    /** min/max Hz are updates by center and span */
    double mMinMHz;
    double mMaxMHz;
    
    /** Required model interface  */
    public double GetXmax(){
        return(mMaxMHz);
    }
    
    /** Required model interface  */
    public double GetXmin(){
        return(mMinMHz);
    }
    
    /**
     * mFcenterMHz - center of frequency sweep requested in MHz
     */
    String mFcenterMHzStr;
    double mFcenterMHz;
    public void setFcenterMHzStr( String newStr ){
        String prevStr = mFcenterMHzStr;
        mFcenterMHz    = Double.parseDouble(newStr);
        mFcenterMHzStr = newStr;
        mPcs.firePropertyChange("FcenterMHzStr",prevStr,newStr);
               
        mMinMHz = (mFcenterMHz - (mSpanMHz/2) );
        mMaxMHz = (mFcenterMHz + (mSpanMHz/2) );
        mSaEngine.SetMinHz(1e6 * mMinMHz);
        mSaEngine.SetMaxHz(1e6 * mMaxMHz);
    }
    public String getFcenterMHzStr(){
        return( mFcenterMHzStr );
    }

    /**
     * mSpanMHzStr - extent of frequency sweep requested in MHz
     */
    String mSpanMHzStr;
    double mSpanMHz;
    public void setSpanMHzStr( String newStr ){
        String prevStr = mSpanMHzStr;
        mSpanMHz     = Double.parseDouble(newStr);
        mSpanMHzStr  = newStr;
        mPcs.firePropertyChange("SpanMHzStr",prevStr,newStr);
        
        mMinMHz = (mFcenterMHz - (mSpanMHz/2) );
        mMaxMHz = (mFcenterMHz + (mSpanMHz/2) );
        mSaEngine.SetMinHz(1e6 * mMinMHz);
        mSaEngine.SetMaxHz(1e6 * mMaxMHz);
    }
    public String getSpanMHzStr(){
        return( mSpanMHzStr );
    }
    
    /**
     * mTgPoints - Number of tracking generator points
     */
    String mTgPointsStr;
    int    mTgPoints;
    public void setTgPointsStr( String newStr ){
        String prevStr = mTgPointsStr;
        mTgPointsStr  = newStr;
        mTgPoints     = Integer.parseInt(mTgPointsStr);
        mPcs.firePropertyChange("TgPointsStr",prevStr,newStr);
        mSaEngine.SetTgPoints(mTgPoints); 
    }
    public String getTgPointsStr(){
        return( mTgPointsStr );
    }
    
    /**
     * mIfGain - IF gain setting
     */
    String mIfGainStr;
    int    mIfGain;
    public void setIfGainStr( String newStr ){
        String prevStr = mIfGainStr;
        mIfGainStr  = newStr;
        mIfGain     = Integer.parseInt(mIfGainStr);
        mPcs.firePropertyChange("IfGainStr",prevStr,newStr);
        mSaEngine.SetIfGain(mIfGain);  
    }
    public String getIfGainStr(){
        return( mIfGainStr );
    }
    
    /**
     * mRfCh - RF channel to use
     */
    String mRfChStr;
    int    mRfCh;
    public void setRfChStr( String newStr ){
        String prevStr = mRfChStr;
        mRfChStr  = newStr;
        mRfCh     = Integer.parseInt(mRfChStr);
        mPcs.firePropertyChange("RfChStr",prevStr,newStr);
        mSaEngine.SetRfCh(mRfCh);
    }
    public String getRfChStr(){
        return( mRfChStr );
    }
    
    /**
     * mMbw - Measurement bandwidth
     */
    String mMbwStr;
    int    mMbw;
    public void setMbwStr( String newStr ){
        String prevStr = mMbwStr;
        mMbwStr  = newStr;
        mMbw     = Integer.parseInt(mMbwStr);
        mPcs.firePropertyChange("MbwStr",prevStr,newStr);
        mSaEngine.SetMbw(mMbw);  
    }
    public String getMbwStr(){
        return( mMbwStr );
    }
    
    /**
     * mFftn - Number of fft points to use
     */
    String mFftnStr;
    int    mFftn;
    public void setFftnStr( String newStr ){
        String prevStr = mFftnStr;
        mFftnStr  = newStr;
        mFftn     = Integer.parseInt(mFftnStr);
        mPcs.firePropertyChange("FftnStr",prevStr,newStr);
        mSaEngine.SetFftn(mFftn);
    }
    public String getFftnStr(){
        return( mFftnStr );
    }

    /**
     * mAve - Number of averages to use
     */
    String mAveStr;
    int    mAve;
    public void setAveStr( String newStr ){
        String prevStr = mAveStr;
        mAveStr  = newStr;
        mAve     = Integer.parseInt(mAveStr);
        mPcs.firePropertyChange("AveStr",prevStr,newStr);
        mSaEngine.SetAve(mAve);
    }
    public String getAveStr(){
        return( mAveStr );
    }
    
    /**
     * mInt - Integration flag
     */
    String mIntStr;
    int    mInt;
    public void setIntStr( String newStr ){
        String prevStr = mIntStr;
        mIntStr  = newStr;
        if( 0==mIntStr.compareTo("ON") ){
            mInt = 1;
        }
        else{
            mInt = 0;
        }    
        mPcs.firePropertyChange("IntStr",prevStr,newStr);
        mSaEngine.SetInt(mInt);
    }
    
    public String getIntStr(){
        return( mIntStr );
    }
    
   /**
     * mAttenc - Coarse attenuation value
     */
    String mAttencStr;
    double mAttenc;
    public void setAttencStr( String newStr ){
        String prevStr = mAttencStr;
        mAttencStr  = newStr;
        mAttenc     = Integer.parseInt(mAttencStr);
        mPcs.firePropertyChange("AttencStr",prevStr,newStr);
        mSaEngine.SetAtten(mAttenc+mAttenf);
    }
    public String getAttencStr(){
        return( mAttencStr );
    }

    /**
     * mAttenf - Fine attenuation value
     */
    String mAttenfStr;
    double mAttenf;
    public void setAttenfStr( String newStr ){
        String prevStr = mAttenfStr;
        mAttenfStr  = newStr;
        mAttenf     = Double.parseDouble(mAttenfStr);
        mPcs.firePropertyChange("AttencStr",prevStr,newStr);
        mSaEngine.SetAtten(mAttenc+mAttenf);
    }
    public String getAttenfStr(){
        return( mAttenfStr );
    }
    
    /**
     * mInputStr - Network device address and port (e.g. 192.168.0.2:8353)
     */
    public void setDevStr( String newStr ){
        mPcs.firePropertyChange("DevStr",mSaEngine.GetInputStr(),newStr);
        mSaEngine.SetInputStr( newStr );    
    }
    public String getDevStr(){
        return( mSaEngine.GetInputStr() );
    }
    
    public void Exit(){
        mSaEngine.Exit();
    }
            
    public void Start(){
        mSaEngine.start();
    }
    
    public void UiUpdateRequest(){
        if( mStateOn ){
            setStatusStr( mSaEngine.getErrStr() );
        }
        else{
            setStatusStr( "---" );
        }
    }
        
    private double mReportRbwHz = 0;
    public String GetReportStr1(){
        return("Fc(MHz)="+mFcenterMHzStr+
               " Span(MHz)="+mSpanMHz+
               " Ref="+mSaEngine.GetRefStr()+
               " ScanHist="+mSaEngine.GetScanCount()+
               " ScanTime(mS)=" +mSaEngine.GetMsPerScan() );
    }
    public String GetReportStr2(){
        return("RBW(Hz)="+(int)(mReportRbwHz) +
               " TGP="+mTgPoints+
               " IFG="+mIfGain+
               " MBW="+mMbw+
               " FFT="+mFftn+
               " AVE="+mAve+
               " INT="+mInt+
               " RFC="+mRfCh+
               " ATN="+(mAttenc+mAttenf)
                );
    }
}
