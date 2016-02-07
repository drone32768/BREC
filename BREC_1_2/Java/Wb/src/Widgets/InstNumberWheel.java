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
package Widgets;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Insets;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.event.MouseWheelListener;
import java.io.Serializable;

/**
 * This class implements a simple number wheel with a 7 segment LED display 
 * look.  It is designed to work as an instrumentation button.
 */
public class InstNumberWheel 
  extends InstWidget // extends javax.swing.JPanel 
  implements Serializable, MouseListener, MouseMotionListener, MouseWheelListener  {
    
    static final long serialVersionUID = 42L;

    /**
     * This nested class is a utility class for drawing digits, accounting
     * for their value and place value.
     */
    public class Digit {
        private char mDigit;
        public void setDigit( char ch ){
            mDigit = ch;
        }
        
        public char getDigit(){
            return(mDigit);
        }
        
        private long mPlaceValue;
        public void setPlace( int p ){
            mPlaceValue = p;
        }
        
        public long getPlace(){
            return( mPlaceValue );
        }
        
        private boolean mHighLight;
        public void setHighLight( boolean v ){
            mHighLight = v;
        }
                
        private int mX0,mY0,mDx,mDy;
        private BasicStroke mStroke;
        public void setDim( int x0, int y0, int dx, int dy ){
            // System.out.println("dim ch="+mDigit+" "+x0+" "+y0+" "+dx+" "+dy);
            mX0=x0;
            mY0=y0;
            mDx=dx;
            mDy=dy;
            
            int strokeWide = (int)(0.1 * mDx);
            if( strokeWide < 1 ) strokeWide = 1;
            
            mStroke = new BasicStroke(strokeWide,
                              BasicStroke.CAP_ROUND,
                              BasicStroke.JOIN_MITER );
        }
        
        public boolean isIn( int x, int y ){
            if( x>=mX0 && x<(mX0+mDx) && y>=mY0 && y<(mY0+mDy) ) return(true);
            return(false);
        }
        
        public Digit(){
           setDigit( '\0' ); 
           setPlace( 0 );
           setHighLight( false );
        } 
        
        
        /**
         * Segment locations referenced below are as follows: (6 is the middle)
         *      1
         *      -
         *   0 | | 2
         *      -
         *   5 | | 3
         *      -
         *      4
         * 
         * @param g 
         */
        private void doSegment0( Graphics2D g2d ){
            int x0,y0,x1,y1;
            x0 = mX0 + (int)(0.10 * mDx);
            y0 = mY0 + (int)(0.45 * mDy);
            x1 = mX0 + (int)(0.10 * mDx);
            y1 = mY0 + (int)(0.15 * mDy);
            g2d.drawLine(x0, y0, x1, y1);
        }
        private void doSegment1( Graphics2D g2d ){
            int x0,y0,x1,y1;
            x0 = mX0 + (int)(0.15 * mDx);
            y0 = mY0 + (int)(0.10 * mDy);
            x1 = mX0 + (int)(0.85 * mDx);
            y1 = mY0 + (int)(0.10 * mDy);
            g2d.drawLine(x0, y0, x1, y1);
        }
        private void doSegment2( Graphics2D g2d ){
            int x0,y0,x1,y1;
            x0 = mX0 + (int)(0.90 * mDx);
            y0 = mY0 + (int)(0.15 * mDy);
            x1 = mX0 + (int)(0.90 * mDx);
            y1 = mY0 + (int)(0.45 * mDy);
            g2d.drawLine(x0, y0, x1, y1);
        }
        private void doSegment3( Graphics2D g2d ){
            int x0,y0,x1,y1;
            x0 = mX0 + (int)(0.90 * mDx);
            y0 = mY0 + (int)(0.55 * mDy);
            x1 = mX0 + (int)(0.90 * mDx);
            y1 = mY0 + (int)(0.85 * mDy);
            g2d.drawLine(x0, y0, x1, y1);
        }

        private void doSegment4( Graphics2D g2d ){
            int x0,y0,x1,y1;
            x0 = mX0 + (int)(0.85 * mDx);
            y0 = mY0 + (int)(0.90 * mDy);
            x1 = mX0 + (int)(0.15 * mDx);
            y1 = mY0 + (int)(0.90 * mDy);
            g2d.drawLine(x0, y0, x1, y1);
        }
        private void doSegment5( Graphics2D g2d ){
            int x0,y0,x1,y1;
            x0 = mX0 + (int)(0.10 * mDx);
            y0 = mY0 + (int)(0.85 * mDy);
            x1 = mX0 + (int)(0.10 * mDx);
            y1 = mY0 + (int)(0.55 * mDy);
            g2d.drawLine(x0, y0, x1, y1);
        }
        private void doSegment6( Graphics2D g2d ){
            int x0,y0,x1,y1;
            x0 = mX0 + (int)(0.15 * mDx);
            y0 = mY0 + (int)(0.50 * mDy);
            x1 = mX0 + (int)(0.85 * mDx);
            y1 = mY0 + (int)(0.50 * mDy);
            g2d.drawLine(x0, y0, x1, y1);
        }
        
        public void doDrawing(Graphics g) {
            Graphics2D g2d = (Graphics2D) g;

            g2d.setStroke(mStroke); 
            
            if( mHighLight ){
                g2d.drawLine(mX0, mY0+mDy, mX0+mDx, mY0+mDy);
            }
            
            switch( mDigit ){
                case '0':
                    doSegment0(g2d);
                    doSegment1(g2d);
                    doSegment2(g2d);
                    doSegment3(g2d);
                    doSegment4(g2d);
                    doSegment5(g2d);
                    // doSegment6(g2d);
                    break;
                case '1':
                    //doSegment0(g2d);
                    //doSegment1(g2d);
                    doSegment2(g2d);
                    doSegment3(g2d);
                    //doSegment4(g2d);
                    //doSegment5(g2d);
                    //doSegment6(g2d);
                    break;
                case '2': // 1,2,6,5,4
                    //doSegment0(g2d);
                    doSegment1(g2d);
                    doSegment2(g2d);
                    //doSegment3(g2d);
                    doSegment4(g2d);
                    doSegment5(g2d);
                    doSegment6(g2d);
                    break;
                    
                case '3': // 3 = 1,2,3,4,6
                    //doSegment0(g2d);
                    doSegment1(g2d);
                    doSegment2(g2d);
                    doSegment3(g2d);
                    doSegment4(g2d);
                    //doSegment5(g2d);
                    doSegment6(g2d);
                    break;
                case '4': // 4 = 0,2,3,6
                    doSegment0(g2d);
                    //doSegment1(g2d);
                    doSegment2(g2d);
                    doSegment3(g2d);
                    //doSegment4(g2d);
                    //doSegment5(g2d);
                    doSegment6(g2d);
                    break;
                case '5': // 5 = 0,1,3,4,6
                    doSegment0(g2d);
                    doSegment1(g2d);
                    // doSegment2(g2d);
                    doSegment3(g2d);
                    doSegment4(g2d);
                    // doSegment5(g2d);
                    doSegment6(g2d);
                    break;
                case '6': // 6 = 0,1,3,4,5,6
                    doSegment0(g2d);
                    doSegment1(g2d);
                    //doSegment2(g2d);
                    doSegment3(g2d);
                    doSegment4(g2d);
                    doSegment5(g2d);
                    doSegment6(g2d);
                    break;
                case '7': // 7 = 1,2,3
                    //doSegment0(g2d);
                    doSegment1(g2d);
                    doSegment2(g2d);
                    doSegment3(g2d);
                    //doSegment4(g2d);
                    //doSegment5(g2d);
                    //doSegment6(g2d);
                    break;
                case '8': // 8 = all
                    doSegment0(g2d);
                    doSegment1(g2d);
                    doSegment2(g2d);
                    doSegment3(g2d);
                    doSegment4(g2d);
                    doSegment5(g2d);
                    doSegment6(g2d);
                    break;
                case '9': // 9 = 0,1,2,3,6
                    doSegment0(g2d);
                    doSegment1(g2d);
                    doSegment2(g2d);
                    doSegment3(g2d);
                    //doSegment4(g2d);
                    //doSegment5(g2d);
                    doSegment6(g2d);
                    break;
                default:
                    break;
            }
   
        }// End of do draw 

    }
    
    ////////////////////////////////////////////////////////////////////////////
    
    Digit   mDigits[];
    Color   mDigitColor;
    boolean mReadOnly;
    
    /**
     * Creates new form NumberWheel
     */
    public InstNumberWheel() {
        setDigits( 13 );
        setNumber( 123456789 );
        mDigitColor = Color.GREEN;
        mReadOnly   = false;
        
        addMouseListener(this);
        addMouseMotionListener(this);
        addMouseWheelListener(this);
    }

    // Label is not used in a numberwheel
    public void setLabel( String str ){
        //
    }
    public String getLabel(){
        return( "InstNumberWheel" );
    }
    
    public void setReadOnly( boolean ro ){
        mReadOnly = ro;
    }
    public boolean getReadOnly(){
        return(mReadOnly);
    }
    
    /**
     * ValueName is the string which contains the name of property when 
     * reporting value changes.  (i.e. on property change notifications,
     * this string will be used as the property name)
     */
    String mValueName;
    public void setValueName( String str ){
        mValueName = str;
    }
    public String getValueName(){
        return(mValueName);
    }
    
   /**
     * Value is the string which contains the numeric display on the wheel
     * AND is reported as the old/new values in any property update 
     * notifications.
     * 
     * @param str 
     */
    String mValueStr = "0";
    public void setValue( String newStr ){
        String prevStr = Long.toString( getNumber() );    
        if( 0!=newStr.compareTo(prevStr) ){
            long v = Long.parseLong(newStr);
            setNumber(v);
            firePropertyChange(mValueName,prevStr,newStr);
            System.out.println("InstNumberWheel:property change="+mValueName+" old="+prevStr+" new="+newStr); 
        }
    }
    public String getValue(){
        return( mValueStr );
    }
            
    /**
     * Set the number of available digits (including spacing digits)
     * @param n 
     */
    public void setDigits( int n ){
        mDigits = new Digit[n];
        for( int dg=0; dg<mDigits.length; dg++ ){
            mDigits[dg] = new Digit();
            mDigits[dg].setDigit( '\0' );
        }        
    }
    
    public void setDigitColor( Color c ){
        mDigitColor = c;  
    }
 
    /**
     * Gets the current integer value of the wheel
     * @return 
     */
    public long getNumber(){
        long value;
        int m;

        value = 0;
        for( int dg=0; dg<mDigits.length; dg++ ){
            m = mDigits[dg].getDigit() - '0';
            if( m>=0 && m<=9 ){
                value += mDigits[dg].getPlace() * m;
            }
        }        
        return( value );
    }
    
    /**
     * Sets the current integer value of the wheel
     * @param n 
     */
    public void setNumber( long n ){
        int p,pv;
        int dg;
        char ch;
        long places [] = new long[32];
        
        // System.out.println("setNumber="+n);
        p = 0;
        while(p<10){
            places[p] = (n % 10);
            // System.out.println("p="+p+" ->"+places[p]);
            n = n / 10;
            p = p + 1;
        }

        int dc=0;
        p  = 0;
        pv = 1;
        for( dg=(mDigits.length-1); dg>=0; dg--){
           
            if( dc==3 ){
                mDigits[dg].setDigit('\0');
                dc=0;
                continue;
            }
            
            if( p<10 ){
               ch = (char)( '0' + places[p] );
            }
            else {
               ch = '0';
            }
            mDigits[dg].setDigit( ch );
            mDigits[dg].setPlace(pv);
            p++;
            pv = pv * 10;
            dc++;
        }
        repaint();
    }
    
    ///////////////////////////////////////////////////////////////////////////
    @Override
    public void paintComponent(Graphics g) {
        super.paintComponent(g);
        doDrawing(g);
    }

    ///////////////////////////////////////////////////////////////////////////
    public void mousePressed(MouseEvent e) {
    }

    public void mouseReleased(MouseEvent e) {

    }

    public void mouseEntered(MouseEvent e) {

    }

    public void mouseExited(MouseEvent e) {

    }

    public void mouseClicked(MouseEvent e) {

    }

    public void mouseMoved(MouseEvent e) {

        // If we are read only nothing to do
        if( mReadOnly ) return;
        
        // Highlight the digit under the mouse
        for( int dg=0; dg<mDigits.length; dg++ ){
            if( mDigits[dg].isIn( e.getX(), e.getY() )) {
                mDigits[dg].setHighLight( true );
            }
            else{
                mDigits[dg].setHighLight( false );
            }
        } 
        repaint();
    }

    public void mouseDragged(MouseEvent e) {
        // saySomething("Mouse dragged", e);
    }
    
    public void mouseWheelMoved(MouseWheelEvent e) {
        int  dg;
        long p,nv;

        // If we are read only nothing to do
        if( mReadOnly ) return;
        
        p = 0;
        for( dg=0; dg<mDigits.length; dg++ ){
            if( mDigits[dg].isIn( e.getX(), e.getY() )) {
                p = mDigits[dg].getPlace();
            }
        } 
        
        nv = getNumber();
        if( e.getWheelRotation()> 0 ){
            nv = nv - p;
            if( nv < 0 ) nv = 0;
        }
        else{
            nv = nv + p;
        }
        setValue( Long.toString(nv) );
    }
    
    ///////////////////////////////////////////////////////////////////////
    private void doDrawing(Graphics g) {

        Graphics2D g2d = (Graphics2D) g;
        Dimension size = getSize();
        Insets insets  = getInsets();

        // Set the font for all text 
        // g2d.setFont(mFont);

        // Figure out the extents of drawing area
        int w = size.width  - insets.left - insets.right;
        int h = size.height - insets.top  - insets.bottom;

        // Calculate the raw extents (unadjusted)
        int dx = (size.width - insets.left - insets.right)/mDigits.length;
        int dy = size.height - insets.top - insets.bottom;
        int x0 = insets.left;
        int y0 = insets.top;
        
        // Force xy ratio of requested
        dy = dx * 5 / 3;
        // dy = dx * 7 / 5;
        
        // Re-center digit array
        x0 = (size.width - (dx*mDigits.length))/2;
        y0 = (size.height- dy)/2;
        
        // Set the dimensions for each character
        for( int dg=0; dg<mDigits.length; dg++ ){
            mDigits[dg].setDim(x0, y0, dx, dy);
            x0 = x0 + dx;
        }
        
        // Draw each digit
        g2d.setColor(mDigitColor);
        for( int dg=0; dg<mDigits.length; dg++ ){
            mDigits[dg].doDrawing(g);
        }
    }          
}
