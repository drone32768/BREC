/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package Models;

import Xy.XyDisplay;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;

/**
 *
 * @author user
 */
public class XyModel implements ModelInterface {
    private XyDisplay      mXy;
    private double         mXdata[];
    private double         mYdata[];
    private int            mMaxPts;  // can be lowered...
    private int            mDataVer;
    private ModelInterface mXySrcObj;
    private int            mXySrcChnl;
    private transient PropertyChangeSupport mPcs;
    
    public XyModel(){
        mXy     = new XyDisplay();
        mMaxPts = 65536;
        mXdata  = new double[ mMaxPts ];
        mYdata  = new double[ mMaxPts ];
        mDataVer=0;
        mPcs    = new PropertyChangeSupport( this );
        
        setPeakEnableStr("OFF");
        setEnvEnableStr( "OFF");
        setPeakFracStr(  "0.01");
        setYrefStr(      "0");
        setYdivStr(      "10");
    }
    
    public void MaxPoints( int n ){
        if( n > mXdata.length ) n = mXdata.length;
        mMaxPts = n;
    }
    
    // TODO need to figure out how to refresh new values or automate
    public void FireAll(){
        mPcs.firePropertyChange("YrefStr",      "",getYrefStr());
        mPcs.firePropertyChange("YdivStr",      "",getYdivStr());
        mPcs.firePropertyChange("YmaxStr",      "",getYmaxStr());
        mPcs.firePropertyChange("YminStr",      "",getYminStr());
        mPcs.firePropertyChange("EnvEnableStr", "",getEnvEnableStr());
        mPcs.firePropertyChange("PeakEnableStr","",getPeakEnableStr());
        mPcs.firePropertyChange("PeakFracStr",  "",getPeakFracStr());
    }
    
    public void addPropertyChangeListener(  PropertyChangeListener listener ){
        mPcs.addPropertyChangeListener(listener);
    }
    
    public void removePropertyChangeListener(  PropertyChangeListener listener ){
        mPcs.removePropertyChangeListener(listener);
    }  
    
    public XyDisplay getXyDisplay(){
        return(mXy);
    }
    
    public void RegisterXySrc( ModelInterface xySrc, int chnlSrc ){
        mXySrcObj = xySrc;
        mXySrcChnl= chnlSrc;
    }
    
    // Required model interface - this model produces no XY data
    public int GetNewXyVer( int ver ){
        return(0);
    }
    
    // Required model interface - this model produces no XY data
    public int GetNewXyData( int chnl, double [] x, double [] y, int maxPoints ){
        return(0);    
    }
    
    /** Required model interface - this model produces no XY data */
    public double GetXmax(){
        return(0.0);
    }
    
    /** Required model interface - this model produces no XY data */
    public double GetXmin(){
        return(1.0);
    }
    
    // Required model interface - this model produces no meaningfull report
    public String GetReportStr1(){
        return("Rpt1");
    }
    
    // Required model interface - this model produces no meaningfull report
    public String GetReportStr2(){
        return("Rpt2");
    }
    
    public void UiUpdateRequest(){
       int ver,npts;
       
       // See if the source model has newer data than we have
       ver = mXySrcObj.GetNewXyVer( mDataVer );
       if( ver!=mDataVer ){
           
           // Save the data version and get the new data
           mDataVer = ver;
           npts = mXySrcObj.GetNewXyData(mXySrcChnl, mXdata, mYdata, mMaxPts );

           // Inject the report summaries into xy display
           mXy.setRpt1Str( mXySrcObj.GetReportStr1() );
           mXy.setRpt2Str( mXySrcObj.GetReportStr2() );
           
           // Inject the new data into xy display
           // (chnl=0 uses source specified, others use extents of data)
           if( mXySrcChnl > 0 ){
               mXy.setXlimits( mXdata[0],mXdata[npts-1]);
           }
           else{
               mXy.setXlimits( mXySrcObj.GetXmin(), mXySrcObj.GetXmax() );
           }
           mXy.setXyData(  mXdata, mYdata, npts );
           // mXy.setXformat('3');
           mXy.setXformat(',');
           
           // Repaint so the new material is shown
           mXy.repaint();
       }
    }
    
    /**
     * 
     */
    String mYrefStr = "1";
    public void setYrefStr( String newStr ){
        String prevStr = mYrefStr;
        mYrefStr = newStr;
        mPcs.firePropertyChange("YrefStr",prevStr,newStr);
        
        double ymax = Double.parseDouble(mYrefStr);
        double ymin = ymax - (Double.parseDouble(mYdivStr)*mXy.getYmajorTicks());
        mXy.setYlimits(ymin,ymax); 
        mXy.repaint();
    }
    public String getYrefStr(){
        return(mYrefStr);
    }

    /**
     * 
     */
    String mYdivStr = "5";
    public void setYdivStr( String newStr ){
        String prevStr = mYdivStr;
        mYdivStr = newStr;
        mPcs.firePropertyChange("YdivStr",prevStr,newStr);

        double ymax = Double.parseDouble(mYrefStr);
        double ymin = ymax - (Double.parseDouble(mYdivStr)*mXy.getYmajorTicks());
        mXy.setYlimits(ymin,ymax); 
        mXy.repaint();
    }
    public String getYdivStr(){
        return(mYdivStr);
    }
    
    /**
     * 
     */
    String mYmaxStr = "+1";
    public void setYmaxStr( String newStr ){
        String prevStr = mYmaxStr;
        mYmaxStr = newStr;
        mPcs.firePropertyChange("YmaxStr",prevStr,newStr);

        double ymax = Double.parseDouble(mYmaxStr);
        double ymin = Double.parseDouble(mYminStr);
        mXy.setYlimits(ymin,ymax); 
        mXy.repaint();
    }
    public String getYmaxStr(){
        return(mYmaxStr);
    }
    
    /**
     * 
     */
    String mYminStr = "-1";
    public void setYminStr( String newStr ){
        String prevStr = mYminStr;
        mYminStr = newStr;
        mPcs.firePropertyChange("YminStr",prevStr,newStr);

        double ymax = Double.parseDouble(mYmaxStr);
        double ymin = Double.parseDouble(mYminStr);
        mXy.setYlimits(ymin,ymax); 
        mXy.repaint();
    }

    public String getYminStr(){
        return(mYminStr);
    }
    
    /**
     * 
     */
    String mEnvEnableStr = "NA";
    public void setEnvEnableStr( String newStr ){
        String prevStr = mEnvEnableStr;
        mEnvEnableStr = newStr;
        mPcs.firePropertyChange("EnvEnableStr",prevStr,newStr);

        if( 0==mEnvEnableStr.compareTo("ON") ){
            mXy.setEnvActive(true);
            mXy.setEnvVisible(true);
        }
        else{
            mXy.setEnvActive(false);
            mXy.setEnvVisible(false);
        }
    }

    public String getEnvEnableStr(){
        return(mEnvEnableStr);
    }
    
        
    /**
     * 
     */
    String mPeakEnableStr = "NA";
    public void setPeakEnableStr( String newStr ){
        String prevStr = mPeakEnableStr;
        mPeakEnableStr = newStr;
        mPcs.firePropertyChange("PeakEnableStr",prevStr,newStr);

        if( 0==mPeakEnableStr.compareTo("ON") ){
            mXy.SetPeakPickCount( 9 );
        }
        else{
            mXy.SetPeakPickCount(0);
        }
    }

    public String getPeakEnableStr(){
        return(mPeakEnableStr);
    }
    
    /**
     * 
     */
    String mPeakFracStr = "NA";
    public void setPeakFracStr( String newStr ){
        String prevStr = mPeakFracStr;
        mPeakFracStr = newStr;
        mPcs.firePropertyChange("PeakFracStr",prevStr,newStr);

        mXy.setPeakFrac( Double.parseDouble(mPeakFracStr) );  
    }

    public String getPeakFracStr(){
        return(mPeakFracStr);
    }
}
