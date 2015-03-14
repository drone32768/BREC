package Models;

import java.io.Serializable;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;

/**
 *
 * @author jkleine
 */
public class SgModel implements ModelInterface, Serializable {

    static final long serialVersionUID = 42L;

    /**
     * This object is the underlying engine of the model.
     */
    private transient SgEngine              mSgEngine;
    
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
    
    public SgModel(){
        mSgEngine = new SgEngine();
        mPcs      = new PropertyChangeSupport( this );
        
        setStateStr(      "OFF");
        setStatusStr(     "---");
        setDevStr(        "192.168.0.2:6786");
        setAux0Str(       "OFF");
        setAux1Str(       "OFF");
        setPowerStr(      "ON");
        setModeStr(       "B");
    }

    
    // Xy data output stream version indicator
    public int GetNewXyVer( int ver ){
        return( 0 );
    }
    
    // Xy data output retrieval
    public int GetNewXyData( int chnl, double [] x, double [] y, int maxPoints ){
        return( 0 );
    }
    
    public String GetReportStr1(){
        return("N/A");
    }
    public String GetReportStr2(){
        return("N/A");
    }
    
    /** Required model interface - this model produces no XY data */
    public double GetXmax(){
        return(0.0);
    }
    
    /** Required model interface - this model produces no XY data */
    public double GetXmin(){
        return(1.0);
    }
    
    public void FireAll(){
        mPcs.firePropertyChange("StateStr",      "",getStateStr());
        mPcs.firePropertyChange("StatusStr",     "",getStatusStr());
        mPcs.firePropertyChange("DevStr",        "",getDevStr());
        mPcs.firePropertyChange("FreqStr",       "",getFreqStr());
        mPcs.firePropertyChange("AttenStr",      "",getAttenStr());
        mPcs.firePropertyChange("Aux0Str",       "",getAux0Str());
        mPcs.firePropertyChange("Aux1Str",       "",getAux1Str());
        mPcs.firePropertyChange("PowerStr",      "",getPowerStr());
        mPcs.firePropertyChange("ModeStr",      "",getPowerStr());
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
            mSgEngine.SetRun(true);
            setFreqStr( getFreqStr() );   // Force device setting
            setAttenStr( getAttenStr() ); // Force deivce setting

        }
        else{
            mStateOn = false;
            mSgEngine.SetRun(false);
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
     * mInputStr - Network device address and port (e.g. 192.168.0.2:8353)
     */
    public void setDevStr( String newStr ){
        mPcs.firePropertyChange("DevStr",mSgEngine.GetInputStr(),newStr);
        mSgEngine.SetInputStr( newStr );    
    }
    public String getDevStr(){
        return( mSgEngine.GetInputStr() );
    }
    
    /**
     * 
     */
    String mFreqStr = "10750000";
    public void setFreqStr( String newStr ){
        mPcs.firePropertyChange("FreqStr"," ",newStr);
        mSgEngine.SetFreq( Long.parseLong(newStr) );   
    }
    public String getFreqStr(){
        return( mFreqStr );
    }
    
    /**
     * 
     */
    String mAttenStr = "0";
    public void setAttenStr( String newStr ){
        mPcs.firePropertyChange("AttenStr"," ",newStr);
        mSgEngine.SetAtten( Long.parseLong(newStr) );    
    }
    public String getAttenStr(){
        return( mAttenStr );
    }
    
    /**
     * 
     */
    String mAux0Str = "OFF";
    public void setAux0Str( String newStr ){
        mPcs.firePropertyChange("Aux0Str",mAux0Str,newStr);
        mAux0Str = newStr;
        if( 0==mAux0Str.compareTo("ON") ){
            mSgEngine.SetAux0(1);    
        }
        else{
            mSgEngine.SetAux0(0); 
        }
            
    }
    public String getAux0Str(){
        return( mAux0Str );
    }

    /**
     * 
     */
    String mAux1Str = "OFF";
    public void setAux1Str( String newStr ){
        mPcs.firePropertyChange("Aux1Str",mAux1Str,newStr);
        mAux1Str = newStr;
        if( 0==mAux1Str.compareTo("ON") ){
            mSgEngine.SetAux1(1);    
        }
        else{
            mSgEngine.SetAux1(0); 
        }
            
    }
    public String getAux1Str(){
        return( mAux1Str );
    }
    
    /**
     * 
     */
    String mPowerStr = "B+C";
    public void setPowerStr( String newStr ){
        mPcs.firePropertyChange("PowerStr",mPowerStr,newStr);
        mPowerStr = newStr;
        if( 0==mPowerStr.compareTo("ON") ){
            mSgEngine.SetPower(1);    
        }
        else{
            mSgEngine.SetPower(0); 
        }
            
    }
    public String getPowerStr(){
        return( mPowerStr );
    } 
 
    /**
     * ModeStr - Mode of operation 
     * "B" B board only
     * "C" C board only
     * "B+C" B mixed with C
     * 
     */
    String mModeStr = "ON";
    public void setModeStr( String newStr ){
        mPcs.firePropertyChange("ModeStr",mModeStr,newStr);
        mModeStr = newStr;
        if( 0==mModeStr.compareTo("B") ){
            mSgEngine.SetMode(0);    
        }
        else if( 0==mModeStr.compareTo("C") ){
            mSgEngine.SetMode(1);
        }
        else{
            mSgEngine.SetMode(2); 
        }
            
    }
    public String getModeStr(){
        return( mModeStr );
    } 
    
    public void Exit(){
        mSgEngine.Exit();
    }
            
    public void Start(){
        mSgEngine.start();
    }
    
    /**
     * This method is designed to be called at a frame rate to update model
     * properties (under a UI thread).
     */
    long mLastLo0 = 0;
    long mLastLo1 = 0;
    public void UiUpdateRequest(){
        long lo0,lo1;
        if( mStateOn ){
            setStatusStr( mSgEngine.getErrStr() );
            
            lo0 = mSgEngine.getLo0Hz();
            if( lo0 != mLastLo0 ){
                mPcs.firePropertyChange("Lo0Str"," ",Long.toString(lo0));
                mLastLo0 = lo0;
            }
            
            lo1 = mSgEngine.getLo1Hz();
            if( lo1 != mLastLo1 ){
                mPcs.firePropertyChange("Lo1Str"," ",Long.toString(lo1));
                mLastLo1 = lo1;                
            }
        }
        else{
            setStatusStr( "---" );
        }
    }
            
}
