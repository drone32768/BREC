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
 * This class encapsulates the display of the values of all markers assigned
 * to it. If a marker is active it will have an indicator to the left of it
 * If a marker is visible the values of the marker x and y will be shown. If
 * a marker is not visible a "---" value will be shown.
 */
public class MarkerTable implements Serializable{
    
    static final long serialVersionUID = 42L;

    char   mXnumberFmt = '6';
    char   mYnumberFmt = '6';

    Color mColor;
    public void setColor(Color c) {
        mColor = c;
    }
    public Color getColor(){
        return(mColor);
    }
    
    public void setXnumberFmt( char ch ){ mXnumberFmt=ch; }
    public void setYnumberFmt( char ch ){ mYnumberFmt=ch; }
    
    int mPx0, mPy0, mPx1, mPy1;
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
    
    Markers mMarkers;
    public void setMarkers(Markers markers) {
        mMarkers = markers;
    }
    public Markers getMarkers(){
        return(mMarkers);
    }

    public MarkerTable() {
        mMarkers = null;
    }

    void doDrawing(Graphics g) {
        Graphics2D g2d = (Graphics2D) g;

        g2d.setColor(mColor);

        int py0;
        for (int n = 0; n < mMarkers.getMarkerCount(); n++) {
            /*
             py0 = mPy0 + (40 * n);
             DrawNumberSingle(g2d,mPx0,py0+10, 'd',9,false,n);
             DrawNumberSingle(g2d,mPx0,py0+20, mXnumberFmt,9,false,mMarkers.GetX(n));
             DrawNumberSingle(g2d,mPx0,py0+30, mYnumberFmt,9,false,mMarkers.GetY(n));
             */
            py0 = mPy0 + (30 * n);
            Misc.DrawNumberSingle(g2d, mPx0, py0 + 10, 'd', 9, mMarkers.GetActive(n), n);

            if (!mMarkers.GetVisible(n)) {
                Misc.DrawStr(g2d, mPx0 + 10, py0 + 10, 9, false, "---");
            } else {
                Misc.DrawNumberSingle(g2d, mPx0 + 10, py0 + 10, mXnumberFmt, 9, false, mMarkers.GetX(n));
                Misc.DrawNumberSingle(g2d, mPx0 + 10, py0 + 20, mYnumberFmt, 9, false, mMarkers.GetY(n));
            }
        }
    }
} // End of MarkerTable
