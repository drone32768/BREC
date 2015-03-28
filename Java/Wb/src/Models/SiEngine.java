package Models;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.util.StringTokenizer;

public class SiEngine extends Thread {

    ///////////////////////////////////////////////////////////////////////////  
    int                    mMaxSteps;
    int                    mIdx;
    int                    mXyPoints;
    double[]               mXdataOut;
    double[]               mYdataOut;
    private int            mDataVer; 
    
    private boolean        mNewDataFlag;
    private String         mIpAddrPortStr;
    
    private BufferedReader mCtlBr;
    private PrintWriter    mCtlPw;
    private Socket         mCtlSock;
    private boolean        mDirty;
    
    ///////////////////////////////////////////////////////////////////////////	
    public SiEngine(){

        mMaxSteps = 512*1024;
        mXyPoints = 1;
        mXdataOut = new double[mMaxSteps];
        mYdataOut = new double[mMaxSteps];

        // Mark current status string as ready
        mErrStr = "RDY";
        
        // Default ref string
        mRefStr = "NA";
    }

    ///////////////////////////////////////////////////////////////////////////
    String         mErrStr;
    public String getErrStr(){
        return(mErrStr);
    }
         
    ///////////////////////////////////////////////////////////////////////////
    boolean mbRun=false;
    public void SetRun( boolean run ){
            mbRun = run;
    }

    public boolean GetRun(){
            return(mbRun);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    private void CollectSamples(){
        String   lineBf;
        String[] fields;
        
        try{
            lineBf = mCtlBr.readLine();
            // System.out.println(lineBf);
            
            fields = lineBf.split(" ",10);
            if( 0==fields[0].compareTo("scan") ){
                mScanEndNs   = System.nanoTime();
                mNsPerScan   = (mScanEndNs-mScanStartNs);       
                mScanStartNs = mScanEndNs;
                mScanCount++;
                mIdx      = 0;
            }
            else if( 0==fields[0].compareTo("reset") ){
                mXyPoints = 1;
                mScanCount= 0;
            }
            else if( 0==fields[0].compareTo("ref") ){
                mRefStr = fields[1];
             }
            else{
                mXdataOut[ mIdx ] = Double.valueOf( fields[0] ) / 1.0e6;
                mYdataOut[ mIdx ] = Double.valueOf( fields[1] );
                mNewDataFlag = true;
                mDataVer++;
                // System.out.println( "("+mXdataOut[mIdx]+","+mYdataOut[mIdx]+")");
                mIdx++;
                if( mIdx > mMaxSteps ) mIdx = mMaxSteps;
                if( mIdx > mXyPoints ) mXyPoints = mIdx;
            }
        }
        catch( Exception e){
            System.out.println("CollectSamples e="+e);
        }
    }   
    
    ///////////////////////////////////////////////////////////////////////////
    long mScanEndNs, mScanStartNs;
    long mNsPerScan;
    public long GetMsPerScan(){
        return( mNsPerScan / 1000000 );    
    }
    
    ///////////////////////////////////////////////////////////////////////////
    String mRefStr;
    public String GetRefStr(){
        return( mRefStr );
    }
    
    ///////////////////////////////////////////////////////////////////////////
    long mScanCount;
    public long GetScanCount(){
        return(mScanCount);
    }
   
    ///////////////////////////////////////////////////////////////////////////
    public int    GetXyPoints()   {
        return(mXyPoints);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    public boolean GetXyData( double [] x, double [] y, int maxPoints ){
        int idx;

        if( !mNewDataFlag ) return(false);
        mNewDataFlag = false;

        for(idx=0;idx<mXyPoints;idx++){
            x[idx] = mXdataOut[idx];
            y[idx] = mYdataOut[idx];
        }

        return( true );    
    }
        
    // Preferred
    public int GetNewXyVer( int ver ){
        return( mDataVer );
    }
    
    // Preferred
    public int GetNewXyData( int chnl, double [] x, double [] y, int maxPoints ){
        int idx,max;
        
        max = Math.min(maxPoints, mXyPoints);
        for(idx=0;idx<max;idx++){
            x[idx] = mXdataOut[idx];
            y[idx] = mYdataOut[idx];
        }
        return(max);
    }
    
    public void SetInputStr( String ipAddrPortStr ){
        mIpAddrPortStr = ipAddrPortStr;
    }
    
    public String GetInputStr( ){
        return(mIpAddrPortStr);
    }
    
    int mTgPoints;
    public void SetTgPoints( int tgPoints ){ 
        mTgPoints=tgPoints; 
        mDirty=true;
    }

    int mIfGainN;
    public void SetIfGain( int ifGainN ){
        mIfGainN = ifGainN;
        mDirty=true;
    }
    
    int mRfCh;
    public void SetRfCh( int rfch ){
        mRfCh = rfch;
        mDirty = true;
    }
        
    int mBwKhz;
    public void SetMbw( int mbw ){
       mBwKhz = mbw;
       mDirty = true;      
    }
    
    int mFftN;
    public void SetFftn( int fftn ){
        mFftN = fftn;
        mDirty = true;       
    }  
    
    int mNave;
    public void SetAve( int ave ){
        mNave = ave;
        mDirty = true;         
    } 
    
    int mInt;
    public void SetInt( int integrate ){
        mInt = integrate;
        mDirty = true;         
    } 
    
    double mAttenDb;
    public void SetAtten( double attendb ){
        mAttenDb = attendb;
        mDirty = true;        
    } 
     
    double mMinHz;
    public void SetMinHz( double fMinHz ){
        mMinHz = fMinHz;
        mDirty = true;      
    }
    
    double mMaxHz;
    public void SetMaxHz( double fMaxHz ){
        mMaxHz = fMaxHz;
        mDirty = true;       
    }

    private void Reconfigure(){
        if( null==mCtlPw ) return;
        if( !mDirty ) return;

        mXyPoints = 1;
        mDirty    = false;

        System.out.println("Reconfigure start");
        try{
            mCtlPw.println("fmin-hz "   + mMinHz);  
            mCtlPw.println("fmax-hz "   + mMaxHz);
            mCtlPw.println("tg-points " + mTgPoints);
            mCtlPw.println("if-gain "   + mIfGainN);
            mCtlPw.println("rf-ch "     + mRfCh);
            mCtlPw.println("mbw-khz "   + mBwKhz);
            mCtlPw.println("fftn "      + mFftN);
            mCtlPw.println("avne "      + mNave);
            mCtlPw.println("integrate " + mInt);
            mCtlPw.println("atten "     + mAttenDb);
        }catch( Exception e ){
            System.out.println("ReConfigure e="+e);
        }            
        System.out.println("Reconfigure complete");
    }
 
    ///////////////////////////////////////////////////////////////////////////
    void OpenLine(  String ipAddrPortStr )
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
            mErrStr = "SKER";
        }
        
        try{
            mCtlPw.println("run");
        }
        catch ( Exception e ){
            System.out.println("socket err"+e);
        }
        mIdx = 0;

    }
    
    ///////////////////////////////////////////////////////////////////////////
    void CloseLine(){
        try{
            mCtlPw.println("halt");
        }
        catch ( Exception e ){
            System.out.println("socket err"+e);
        }  
        try{
            mCtlSock.close();
        }
        catch( Exception e ){
            System.out.println("socket close "+e);
        }
        mCtlPw = null;
    }
    
    void Exit(){
        mbRun        = false;
        mbThreadExit = true;
    }
    
    boolean IsExited(){
        return( mbThreadIsExited );
    }
    
    ///////////////////////////////////////////////////////////////////////////
    boolean mbThreadExit     = false;
    boolean mbThreadIsExited = false;
    public void run() {
        System.out.println("SaEngine thread starting");
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
                Reconfigure();
                CollectSamples();
            }
            CloseLine();
                
        }// End of loop over mbThreadExit
        
        mbThreadIsExited = true;
        System.out.println("SaEngine thread exiting");
    } // End of run

}