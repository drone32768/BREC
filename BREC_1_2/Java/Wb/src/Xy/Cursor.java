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
 * This class encapsulates a cursor position along with the x and y axis
 * that define the logical/physical translation.
 */
public class Cursor implements Serializable {
    
    static final long serialVersionUID = 42L;

    private char   mNumberFmt = 'g';

    private double mLcurX;
    private double mLcurY;
    public void   setLcurX(double v ){ mLcurX=v; }
    public double getLcurX()         { return(mLcurX); }
    public void   setLcurY(double v ){ mLcurY=v; }
    public double getLcurY()         { return(mLcurY); }
    
    public double GetX() {
        return (mLcurX);
    }

    public double GetY() {
        return (mLcurY);
    }
     
    private int mPx0, mPy0, mPx1, mPy1;
    public void setPx0(int v){   mPx0=v; }
    public int  getPx0(){ return(mPx0); }
    public void setPx1(int v){   mPx1=v; }
    public int  getPx1(){ return(mPx1); }
    public void setPy0(int v){   mPy0=v; }
    public int  getPy0(){ return(mPy0); }
    public void setPy1(int v){   mPy1=v; }
    public int  getPy1(){ return(mPy1); }
    
    public void SetDrawingExtents(int x0, int y0, int x1, int y1) {
        mPx0 = x0;
        mPy0 = y0;
        mPx1 = x1;
        mPy1 = y1;
    }
    
    private Color mColor;
    public void setColor(Color c) {
        mColor = c;
    }
    public Color getColor(){
        return(mColor);
    }
    
    private Axis mXaxis;
    private Axis mYaxis;
    public void setXaxis( Axis ax ){ mXaxis=ax; }
    public Axis getXaxis()         { return(mXaxis); }
    public void setYaxis( Axis ax ){ mYaxis=ax; }
    public Axis getYaxis()         { return(mYaxis); }
    
    public void SetAxis(Axis xA, Axis yA) {
        mXaxis = xA;
        mYaxis = yA;
    }

    public Cursor() {
        // Default data
        SetDrawingExtents(0, 0, 10, 10);
        mLcurX = 0;
        mLcurY = 0;
    }

    public void Update(int px, int py) {
        mLcurX = mXaxis.PhysicalToLogical(px);
        mLcurY = mYaxis.PhysicalToLogical(py);
    }

    public void doDrawing(Graphics g) {
        Graphics2D g2d = (Graphics2D) g;

        // Do not draw cursor if its beyond axis logical extents
        if( (mLcurX<mXaxis.getLstart()) || (mLcurX>mXaxis.getLend()) ){
            return;
        }
        if( (mLcurY<mYaxis.getLstart()) || (mLcurY>mYaxis.getLend()) ){
            return;
        }        
        
        g2d.setColor(mColor);
        int px,py;
        
        px = mXaxis.LogicalToPhysical(mLcurX);
        py = mYaxis.LogicalToPhysical(mLcurY);
        Misc.DrawNumberSingle(g2d, px, py, mNumberFmt,  3, false, mLcurX);
        Misc.DrawNumberSingle(g2d, px, py, mNumberFmt,  9, false, mLcurY);
        
        /*
        Misc.DrawNumberSingle(g2d, mPx0 + 8, mPy0 + 30, mNumberFmt, 9, false, mLcurX);
        Misc.DrawNumberSingle(g2d, mPx0 + 8, mPy0 + 40, mNumberFmt, 9, false, mLcurY);
        */
        
    }
}
