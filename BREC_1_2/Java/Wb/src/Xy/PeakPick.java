
package Xy;

import java.io.Serializable;

/**
 *
 */
public class PeakPick implements Serializable{
    static final long serialVersionUID = 42L;
    static final int  MAX_PEAKS = 10;
    
    private double[]       mPeaksX;
    public void setPeaksX( double[] ra ){
        mPeaksX = ra;
    }
    public double[] getPeaksX(){
        return(mPeaksX);
    }
    
    private double[]       mPeaksY;
    public void setPeaksY( double[] ra ){
        mPeaksY = ra;
    }
    public double[] getPeaksY(){
        return(mPeaksY);
    }
    
    transient private double[]       mMaskY;
    
    private int            mPeakCount;
    public void setPeakCount( int va ){
        mPeakCount = va;
    }
    public int getPeakCount(){
        return(mPeakCount);
    }
    
    private double         mPeakFrac;
    public void setPeakFrac( double f ){
        mPeakFrac = f;
    }
    public double getPeakFrac(){
        return(mPeakFrac);
    }

    PeakPick(){
       mPeakCount = 0;
       mPeaksX   = new double[ MAX_PEAKS ];
       mPeaksY   = new double[ MAX_PEAKS ];
       mMaskY    = new double[ 8192 ];
       mPeakFrac = 0.01;
    }
    
    
    public double GetPeakX( int idx ){
        return(mPeaksX[idx]);
    }
    public double GetPeakY( int idx){
        return(mPeaksY[idx]);
    }
    
    private void PeakClear( int idx, int step, int clrPts, int npts ){
        int dstIdx = idx;
        for(int count=0;count<clrPts;count++ ){
            if( (dstIdx>=0) && (dstIdx<npts) ){
                mMaskY[dstIdx] = -Double.MAX_VALUE;
                dstIdx = dstIdx + step;
            }
            else{
                return;
            }
        }
    }
        
    void PickPeaks( double [] xarray, double [] yarray, int npts ){
        int    dstIdx,srcIdx,peakIdx,clrPts;

        // Make sure the mask is large enough for this data
        if( mMaskY.length < npts ){
            mMaskY = new double[npts];
        }
        
        // Copy Y output to mask
        for(srcIdx=0;srcIdx<npts;srcIdx++){
            mMaskY[srcIdx] = yarray[srcIdx];    
        }
        
        // Set the number of points around a peak to omit on peak find
        clrPts = (int)(npts * mPeakFrac);
                
        // Loop over the number of peaks we are looking for
        for(dstIdx=0;dstIdx<mPeakCount;dstIdx++){
            
            // For this peak, set min and scan over mask for peak
            mPeaksY[ dstIdx ] = -Double.MAX_VALUE;
            peakIdx = -1;
            for(srcIdx=0;srcIdx<npts;srcIdx++){
               if( mMaskY[srcIdx]>mPeaksY[dstIdx] ){
                   peakIdx = srcIdx;
                   mPeaksX[ dstIdx ] = xarray[srcIdx];
                   mPeaksY[ dstIdx ] = yarray[srcIdx];
               } 
            }// End of loop over src idx
            
            if( peakIdx < 0 ){
                   mPeaksX[ dstIdx ] = mPeaksX[0];
                   mPeaksY[ dstIdx ] = mPeaksY[0];
            }
            
            // Clear the peak just found (and nearby)
            PeakClear( peakIdx,  1, clrPts, npts );
            PeakClear( peakIdx, -1, clrPts, npts );
            
        }// End of loop over dst idx    
        
    }
}
