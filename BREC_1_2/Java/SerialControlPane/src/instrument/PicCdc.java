/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package instrument;

import gnu.io.CommPort;
import gnu.io.CommPortIdentifier;
import gnu.io.SerialPort;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Enumeration;

/*
 * This class provides an interface to PIC processors over USB.
 * 
 * States of an object are:
 * 
 * IDLE
 * PROBING
 * OPEN
 * CLOSED
 * 
 * Status of an object is depends on the state and is not specified in detail
 * There is a status string available indicating the most recent status update
 * in human readable text.
 * 
 * The PIC device will appear as a serial port (via the communications
 * device class or cdc under windows).  Each PIC devices has a set of
 * registers.  Registers are read and written by sending 4 byte command
 * packets and receiving 4 byte command packets.  The format of those are:
 * 
 * CMD
 * 0xA5 <R|W> <addr> <value>
 * 
 * RESP
 * 0xA5 <r|w> <addr> <value>
 * 
 * e.g.
 * buffer[lnDst++] = 0xA5;
 * buffer[lnDst++] = 'R';
 * buffer[lnDst++] = 0x80 + lnIdx;
 * bffer[lnDst++] = 0x00;
 *
 * The common registers for device registration, identification and
 * embedded user interface control are:
 * 
 * 0x80 = 2x16 LCD display (64 bytes)
 * 0xC0 = switch down
 * 0xC1 = switch up
 * 0xC2 = ?
 * 0xC3 = Encoder right
 * 0xC4 = Encoder left
 * 0xC5 = project  number
 * 0xC6 = serial   number
 * 0xC7 = hardware version
 * 0xC6 = software version
 * 
 * NOTE: The project number is compiled in while the serial number is 
 * configurable by writing to eeprom location 59 (0x3B).  The eeprom
 * serial number is only returned if the PIC has eeprom setting use enabled
 * (location 63 (0x3F) must be equal to 1)
 */
public class PicCdc implements Runnable  {
    
    /**
     * This nested class is used internally to support asynchronous opens of a device.
     * The Open may take several seconds (when several devices are present
     * including probing available devices for a specific project and serial id)
     * 
     * To avoid blocking the caller thread, an instance of this object is used
     * to create a separate thread to drive the opening activities.
     */
    public class AsynchOpener implements Runnable {
        PicCdc  mPic;
        boolean mMatch;
        int     mPrjId;
        int     mSnId;
        
        AsynchOpener( PicCdc pic, boolean matchId, int prjId, int snId ){
             mPic   = pic;
             mMatch = matchId;
             mPrjId = prjId;
             mSnId  = snId;
        }

        @Override
        public void run (){
             mPic.Open(mMatch,mPrjId,mSnId); 
        }
    }
          
    /**
     * This is the serial port input stream.
     */
    private InputStream        mInputStream;
    
    /**
     * This is the serial port output stream
     */
    private OutputStream       mOutputStream;
    
    /**
     * This is the java serial package com port handle ( port id )
     */
    private CommPortIdentifier mPortId;
    
    /**
     * This is the java serial package com port handle ( com port )
     */
    private CommPort           mComPort;
    
    /**
     * This is the java serial package com port handle ( serial port )
     */
    private SerialPort         mSerialPort;
    
    /**
     * This input byte buffer is used for reading device responses
     */
    private byte [] mInputProcBf;
        
    /**
     * This is an array of the device registers.  It contains the most recently
     * produced value for that register from the device.
     */
    private int []             mRegs;
    
    /**
     * This is an array of device register update flags.  Every time the 
     * device produces a response regarding a register, this value (at
     * the appropriate register index) is set to 1.
     */
    private int []             mUpd;
    
    /**
     * This is the project id of the device read at open/probe/attach time
     */
    private int                mPrjId;
    
    /** 
     * This is the serial number id of the device read at open/probe/attach time
     */
    private int                mSnId;

    /**
     * The state of the device is either open, probe, idle.  If this 
     * is true then the state is open.
     */
    private boolean            mIsOpen;
    
    /**
     * The state of the device is either open, probe, idle.  If this 
     * is true then the state is probe.
     */
    private boolean            mIsProbing;
    
    /**
     * This is an internal flag used to indicate to supporting threads
     * to continue executing or not.
     */
    private boolean            mDoRun;
   
    /**
     * This string reflects the most recent status of the device.  It
     * is not intended to be parsed, rather, it is a human readable indicator.
     */
    private String             mStatusStr;
    
    /**
     * This is the number of bytes written to the device.
     */
    private int                mWbytes;
    
    /**
     * This is the number of bytes read from the device.
     */
    private int                mRbytes;
    
    /**
     * Constructor.  Only internal memory is established.  The next
     * operation must be an Open() flavor.
     */
    public PicCdc(){
        mRegs        = new int[256];   // TODO - enum these? addr is 8 bits so never larger than that
        mUpd         = new int[256];
        mInputProcBf = new byte[1024];
        mPrjId       = -1;
        mSnId        = -1;
        mIsOpen      = false;
        mIsProbing   = false;
        mStatusStr   = "Created";
    }

    /** True if device is in open state */
    public boolean IsOpen()          { return(mIsOpen);    }
    
    /** True if the device is in the probe state */
    public boolean IsProbing()       { return(mIsProbing); }
    
    /** Returns the current devices project id */
    public int     GetPrjNumber()    { return(mPrjId);     }
    
    /** Returns the current devices serial number */
    public int     GetSerialNumber() { return(mSnId );     }
    
    /** Returns a string describing the current status of this object */
    public String  GetStatusStr()    { return(mStatusStr); }
    
    /** Returns the number of bytes written to the current device */
    public int     GetWriteBytes()   { return(mWbytes);    }
    
    /** Returns the number of bytes read from the current device */
    public int     GetReadBytes()    { return(mRbytes);    }
    
    /** Returns a string for human readable state */
    public String  GetStateStr(){
        if( mIsProbing ) return("PROBE");
        if( mIsOpen    ) return("OPEN");
        return("IDLE");
    }
    
    /** Returns the most recently updated value of the specified register.
     * NOTE: this is only the local copy and may not be accurate depending
     * upon what updates the device has sent (by being accessed) and the
     * current connection state to the device.
     * @param addr
     * @return 
     */
    public int GetReg( int addr ){
        addr = addr%256;
        return( mRegs[addr] );
    }
    
    /**
     * This method initiates an 8 bit read from the open device at the 
     * specified location.  NOTE: this only initiates the read.  The
     * actual results will return from the device in the future and be 
     * available then.  See Rd8Synch().
     * @param addr
     * @return 
     */
    public synchronized int Rd8( int addr ){
        byte [] outb = new byte[4];
        outb[0] = (byte)0xA5;
        outb[1] = (byte)'R';
        outb[2] = (byte)(addr);
        outb[3] = (byte)0x00;
        try{
           this.mOutputStream.write(outb);
           this.mOutputStream.flush();
           mWbytes+=4;
        }catch( Exception e ){
            mStatusStr ="err write R";
            LogMsg(mStatusStr);
            return(-1);
        }       
        return(0);
    }
    
    /**
     * Dual of Rd8.  See that documentation.
     * @param addr
     * @param value
     * @return 
     */
    public synchronized int Wr8( int addr, int value ){
        byte [] outb = new byte[4];
        outb[0] = (byte)0xA5;
        outb[1] = (byte)'W';
        outb[2] = (byte)(addr);
        outb[3] = (byte)(value&0xff);
        try{
           this.mOutputStream.write(outb);
           this.mOutputStream.flush();
           mWbytes+=4;
        }catch( Exception e ){
            mStatusStr ="err write W";
            LogMsg(mStatusStr);
            return(-1);
        }
        return(0);
    }
    
    /**
     * This method initiates a read of the specified address and waits
     * until results for that address have been returned.  If those results
     * are returned within a reasonable and acceptable time they are returned
     * else a -1 is.
     * @param addr
     * @return 
     */
    public synchronized int Rd8Synch( int addr ){
        int waitCount=40;
        
        mUpd[ addr ] = 0;
        Rd8( addr );
        
        while( (0==mUpd[addr]) && (waitCount>0) ){
            try{
               Thread.sleep(2);
            }
            catch( Exception e ){
                ;
            }
            waitCount--;
        } 
        if( waitCount <=0 ){
            return(-1);
        }
        else{
            return( mRegs[addr] );    
        }     
    }

    /**
     * This method evaluates opens a com port for register access to a device.
     * If the matchIds argument is false, then the first available com port
     * is used.  If the match id argument is true, then each openable com port
     * is probe for a device project and serial number id.  If they match
     * those specified the device is kept open a 0 is returned.  If no
     * matching devices can be opened a number less than zero is returned.
     * 
     * NOTE: This operation may take several seconds.  See OpenAsynch if the
     * caller cannot block for such times.
     * 
     * @param matchIds
     * @param prjId
     * @param snId
     * @return 
     */
    public synchronized int Open( boolean matchIds, int prjId, int snId ){
        String portName;
        int    err;
        int    cn;
        
        if( mIsOpen ){
            return(-2);
        }
        
        mIsProbing = true;
        mStatusStr = "PortQuery";
        mRbytes    = 0;
        mWbytes    = 0;
        mDoRun     = true;
        
        // Loop over all available com ports
        Enumeration thePorts = CommPortIdentifier.getPortIdentifiers();
        while (mDoRun && thePorts.hasMoreElements()) {
            
            //  Make sure this is a serial port
            CommPortIdentifier com = (CommPortIdentifier) thePorts.nextElement();
            if( com.getPortType() != CommPortIdentifier.PORT_SERIAL ) continue;
            
            // Skip COM1 and COM2
            LogMsg("Port, " + com.getName() + " is avail");
            portName = com.getName();
            if( 0==portName.compareTo("COM1") || 0==portName.compareTo("COM2") ) continue;
            
            // For testing purpose it is sometimes usefull to allow COM3
            // Comment out the following line.
            if( 0==portName.compareTo("COM3")) continue;
           
            // Try to attach
            err = Attach(portName);
            
            // If we did not successfully attach move to next
            if( err!= 0 ){
                continue;
            }
            
            // If we did attach and we are matching ids but the id's are wrong move to next
            if( matchIds && ((mPrjId!=prjId) || (mSnId!=snId)) ){
                Dettach();
                continue;
            }
            
            // We attached ok and the id's match (if required) so complete the open
            mIsOpen    = true;
            mIsProbing = false;
            mStatusStr = portName+" OK";

            new Thread( this ).start(); 
            return( 0 );
        }
        
        mIsProbing = false;
        mStatusStr = "Device Not Found";
        
        // No device attached successfully and matched ids
        return( -1 );    
    }

    /**
     * This method performs an open operation but uses an external thread
     * to conduct the operation. (i.e. the caller will return immediately,
     * however, the device may not be open.  The state of this objects
     * open/probe flags must be consulted to determine useability and access
     * to the device).
     * 
     * @param matchIds
     * @param prjId
     * @param snId
     * @return 
     */
    public synchronized int OpenAsynch( boolean matchIds, int prjId, int snId ){
        AsynchOpener       mOpener;
        mOpener = new AsynchOpener( this, matchIds, prjId, snId );
        new Thread( mOpener ).start();
        return(0);
    }
    
    /**
     * This method indicates close of this objects access to the current device.
     * NOTE: return does not indicate full closure, only that closure has
     * been initiated.  The open state and probe state will be false (i.e. idle)
     * 
     * @return 
     */
    public synchronized int Close(){
        mDoRun = false;
        return(0);
    }
       
    /**
     * This is the internal thread method.  It should not be invoked externally
     * It is responsible for processing data from the device in a meaningfull
     * manner to this objects internals.
     */
    @Override
    public void run (){

        int bytesRead;
        
        LogMsg("Internal input thread starting");

        do{
            bytesRead = ProcessInput();
        }while( (bytesRead > -1) && mDoRun );
        
        mIsOpen = false;
        Dettach();
        
        LogMsg("Internal input thread exiting");
    }
    
    /**
     * This internal method attempts to open the com port specified by name.
     * If the port can be opened the device on the port has its project id
     * and serial number read and updated within the object.
     * 
     * @param portName
     * @return 0 indicate success/no error. All other values are error codes
     */
    private int Attach( String portName ){
        LogMsg("Attaching to "+portName);
        mStatusStr = "Attaching "+portName;
                
        try{
            mPortId = CommPortIdentifier.getPortIdentifier(portName);
        }catch( Exception e ){
            mStatusStr ="err getport";
            LogMsg(mStatusStr);
            return( -3 );
        }
        
        if ( mPortId.isCurrentlyOwned() )
        {
            mStatusStr ="err busy";
            LogMsg(mStatusStr);
            return( -1 );
        }

        LogMsg("Opening "+portName);
        try {
            mComPort = mPortId.open(this.getClass().getName(),0);
        }catch( Exception e ){
            mStatusStr ="err get type";
            LogMsg(mStatusStr);
            return( -100 );
        }

        if ( !(mComPort instanceof SerialPort) )
        {
            mStatusStr ="err not com";
            LogMsg(mStatusStr);
            return(-2);
        }
        
        try {
            mSerialPort = (SerialPort) mComPort;
            mSerialPort.enableReceiveTimeout( 100 ); // 100mS second rcv timeout
        }catch( Exception e ){
            mStatusStr ="err set tout";
            LogMsg(mStatusStr);
            return(-100);
        }
        
        try{
            mInputStream  = mSerialPort.getInputStream();
            mOutputStream = mSerialPort.getOutputStream();
        }catch( Exception e ){
            mStatusStr ="err get streams";
            LogMsg(mStatusStr);
        }
        
        LogMsg("Identifying device");
        // Read the project and serial number
        // FIXME - this assumes queue is empty
        Rd8( 0xC5 );
        Rd8( 0xC6 );
        int sumBytes =0;
        int loopCount=8;
        while( (sumBytes<8) && (loopCount>0) ){
            sumBytes += ProcessInput();
            loopCount--;
        }
        LogMsg("Project number = "+mRegs[0xC5]);
        LogMsg("Serial  number = "+mRegs[0xC6]);
        mPrjId = mRegs[0xC5];
        mSnId  = mRegs[0xC6];
        
        LogMsg( "Attach successful" );
        return(0);
    }
    
    /**
     * This internal method closes the open serial port.
     * @return 
     */
    private int Dettach()
    {
        LogMsg("Dettaching");
        mStatusStr ="Closing";
        mSerialPort.close();
        mStatusStr ="Closed";
        LogMsg("Dettach Complete.");
        return(0);
    }
    
    /**
     * This internal method reads the input stream, parses and processes it
     * and returns the number of input bytes from the device processed.
     */
    private int ProcessInput(){
        int nBytes=0;
        try{
            nBytes = this.mInputStream.read( mInputProcBf ); 
            mRbytes += nBytes;
        }
        catch( IOException e ){
            ;
        }
        ParseInputBytes( mInputProcBf, nBytes );
        return(nBytes);
    }
    
    /** Number of currently parsed bytes */
    private int     mParsedBytes = 0;
    /** Parsed r/w flag.  True if parsed a "read" */
    private boolean mParsedR;
    /** Parsed address */
    private int     mParsedA;
    /** Parsed value */
    private int     mParsedV;

    /**
     * This method accepts an array of input bytes from the device, parses them
     * and updates the local copy of registers/register update flags 
     * (mRegs/mUpd).
     * 
     * @param inputBf
     * @param inputBytes 
     */
    private void ParseInputBytes( byte[] inputBf, int inputBytes ){
        
        for( int idx=0; idx<inputBytes; idx++ ){
            switch( mParsedBytes ){
                case 0 :
                    if( inputBf[idx] == ((byte)0xA5) ){
                        mParsedBytes++;
                    }
                    break;
                case 1 :
                    if( inputBf[idx]==((byte)'r') || inputBf[idx]==((byte)'w') ){
                        mParsedR = inputBf[idx]=='r';
                        mParsedBytes++;
                    }
                    else{
                        // NOTE: This has the effect that if the deserializer
                        // begins at the wrong spot in the input stream (i.e.
                        // there were residual bytes queued, we will continually
                        // reset parsing until a value 0xA5 ['r'|'w'] is
                        // received and then resynchronize to the parsing
                        mParsedBytes = 0;
                    }
                    break;
                case 2 :
                    mParsedA = (inputBf[idx])&0xff;
                    mParsedBytes++;
                    break;
                case 3 :                   
                    mParsedV     = (inputBf[idx])&0xff;
                    mParsedBytes = 0;
                    if( mParsedR ){
                        mRegs[ mParsedA ] = mParsedV;
                        mUpd[  mParsedA ] = 1;
                    }
                    break;
            }
        }
    }
    
    /**
     * This is an internal method that logs a string with timestamp prefix
     * @param str 
     */
    private void LogMsg( String str ){
        System.out.println( System.currentTimeMillis()+":"+str);
    }
}
