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

package Xy;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.io.Serializable;


/**
 * The Axis class provides two basic capabilities: a) Drawing of an axis b)
 * Conversion from logical to physical coordinates.
 */
public class Axis implements Serializable{

    static final long serialVersionUID = 42L;
    
    char   mNumberFmt = 'g';
    public void setNumberFormat( char fmtCode ){
        mNumberFmt = fmtCode;
    }
    public char getNumberFormat(){
        return( mNumberFmt );
    }

    String mLabelStr;
    public void setLabelStr(String s) {
        mLabelStr = s;
    }
    public String getLabelStr(){
        return(mLabelStr);
    }
    
    int mMajorTickPixels;
    int mNmajorTicks;
    public void setMajorTicks(int n) {
        mNmajorTicks = n;
    }
    public int getMajorTicks(){
        return(mNmajorTicks);
    }
    
    int mMinorTickPixels;
    int mNminorTicks;
    public void setMinorTicks(int n) {
        mNminorTicks = n;
    }
    public int getMinorTicks(){
        return(mNminorTicks);
    }
    
    Color mColor;
    public void setColor(Color c) {
        mColor = c;
    }
    public Color getColor(){
        return(mColor);
    }
    
    double mLstart;
    public void setLstart(double v){ mLstart=v; }
    public double getLstart(){ return(mLstart); }
    
    double mLend;
    public void setLend(double v){ mLend=v; }
    public double getLend(){ return(mLend); }
    
    double mLdelta;
    public void setLdelta(double v){ mLdelta=v; }
    public double getLdelta(){ return(mLdelta); }
    
    public void SetLogicalExtents(double s, double e) {
        mLstart = s;
        mLend   = e;
        mLdelta = mLend - mLstart;
    } 

    boolean mIsHorizontal;
    public void setHorizontal(boolean v){ mIsHorizontal=v; }
    public boolean getHorizontal(){ return(mIsHorizontal); }
    
    int mPx0, mPy0, mPx1, mPy1;
    public void setPx0(int v){   mPx0=v; }
    public int  getPx0(){ return(mPx0); }
    public void setPx1(int v){   mPx1=v; }
    public int  getPx1(){ return(mPx1); }
    public void setPy0(int v){   mPy0=v; }
    public int  getPy0(){ return(mPy0); }
    public void setPy1(int v){   mPy1=v; }
    public int  getPy1(){ return(mPy1); }
    
    int mPdelta;
    public void setPdelta(int v){ mPdelta=v; }
    public int getPdelta(){ return(mPdelta); }
    
    int mPstart,mPend;
    public void setPstart(int v){ mPstart=v; }
    public int getPstart(){ return(mPstart); }
    public void setPend(int v){ mPend=v; }
    public int getPend(){ return(mPend); }
    
    public void SetDrawingExtents(boolean h, int x0, int y0, int x1, int y1) {
        mIsHorizontal = h;
        mPx0 = x0;
        mPy0 = y0;
        mPx1 = x1;
        mPy1 = y1;
        if (mIsHorizontal) {
            mPdelta = mPx1 - mPx0;
            mPstart = mPx0;
            mPend   = mPx1;
        } else {
            mPdelta = mPy1 - mPy0;
            mPstart = mPy0;
            mPend   = mPy1;
        }
    }
    public int GetPhysicalMin(){
        return(mPstart);
    }
    public int GetPhysicalMax(){
        return(mPend);
    }

    /**
     * No arg constructor.
     */
    public Axis() {
        // Default data
        mMajorTickPixels = 8;
        setLabelStr("label");
        SetLogicalExtents(0.0, 1.0);         // Provide default logical extents
        SetDrawingExtents(true, 0, 0, 10, 10);  // Provide default drawing extents
        setMajorTicks(5);
        setMinorTicks(7);
    }

    /**
     * Takes a logical (real value) and returns a physical device (drawing
     * value)
     *
     * @param logical value
     * @return physical value
     */
    public int LogicalToPhysical(double v) {
        int n = mPstart + (int) (mPdelta * ((v - mLstart) / mLdelta));
        return (n);
    }

    /**
     * Takes a physical (device coordinate) value and returns a logical
     * (real value)
     *
     * @param real value
     * @return logical value
     */
    public double PhysicalToLogical(int n) {
        double r = mLstart + (mLdelta * (double) (n - mPstart) / mPdelta);
        return (r);
    }

    /**
     * Painting/Drawing method
     *
     * @param Graphics context to conduct drawing in
     */
    void doDrawing(Graphics g) {
        Graphics2D g2d = (Graphics2D) g;

        // Set the axis color
        g2d.setColor(mColor);

        // Draw the axis itself
        g2d.drawLine(mPx0, mPy0, mPx1, mPy1);

        // Draw the major ticks and labels
        int lnIdx, px1, py1, px2, py2;
        double r = mLstart;
        for (lnIdx = 0; lnIdx <= mNmajorTicks; lnIdx++) {
            if (mIsHorizontal) {
                px1 = LogicalToPhysical(r);
                py1 = mPy0;
                py2 = py1 + mMajorTickPixels;
                Misc.DrawNumberSingle(g2d, px1, py2, mNumberFmt, 12, false, r);
                g2d.drawLine(px1, py1, px1, py2);
            } else {
                py1 = LogicalToPhysical(r);
                px1 = mPx1;
                px2 = px1 - mMajorTickPixels;
                Misc.DrawNumberSingle(g2d, px2, py1, mNumberFmt, 3, false, r);
                g2d.drawLine(px1, py1, px2, py1);
            }
            r = r + mLdelta / mNmajorTicks;
        }

        // Draw the axis label
        if (mIsHorizontal) {
            Misc.DrawStr(g2d, (mPx0 + mPx1) / 2, mPy0 + 20, 12, false, mLabelStr);
        } else {
            Misc.DrawStr(g2d, mPx1 - 20, (mPy0 + mPy1) / 2, 3, false, mLabelStr);
        }

    }// End of doDrawing
} // End of Axis

