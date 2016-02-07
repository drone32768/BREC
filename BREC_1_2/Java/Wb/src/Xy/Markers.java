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

 ///////////////////////////////////////////////////////////////////////////	
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.io.Serializable;

/**
 * This class holds a fixed number of xy markers along with the x and y axis
 * needed for logical/physical translation. Each marker can be shown or
 * hidden. Each marker can have its x and y values manipulated.
 */
public class Markers implements Serializable{
    
    static final long serialVersionUID = 42L;
    
    double[] mMarkerX;
    public void setMarkerX( double [] ra ){
        mMarkerX = ra;
    }
    public double [] getMarkerX(){
        return(mMarkerX);
    }
    
    double[] mMarkerY;
    public void setMarkerY( double [] ra ){
        mMarkerY = ra;
    }
    public double [] getMarkerY(){
        return(mMarkerY);
    }
    
    boolean[] mMarkerActive;
    public void setMarkerActive( boolean [] ba ){
        mMarkerActive = ba;
    }
    public boolean [] getMarkerActive(){
        return(mMarkerActive);
    }
    
    boolean[] mMarkerVisible;
    public void setMarkerVisible( boolean [] ba ){
        mMarkerVisible = ba;
    }
    public boolean [] getMarkerVisible(){
        return(mMarkerVisible);
    }
    
    Color mColor;
    public void setColor(Color c) {
        mColor = c;
    }
    public Color getColor(){
        return(mColor);
    }

    int mMarkerCount;
    public void setMarkerCount( int n ){
        mMarkerCount = n;
    }
    public int getMarkerCount() {
        return (mMarkerCount);
    }

    public boolean GetVisible(int idx) {
        return (mMarkerVisible[idx]);
    }

    public void SetVisible(int idx, boolean on) {
        mMarkerVisible[ idx] = on;
    }

    public double GetX(int idx) {
        return (mMarkerX[idx]);
    }

    public void SetX(int idx, double r) {
        mMarkerX[idx] = r;
    }

    public double GetY(int idx) {
        return (mMarkerY[idx]);
    }

    public void SetY(int idx, double r) {
        mMarkerY[idx] = r;
    }

    public boolean GetActive(int idx) {
        return (mMarkerActive[idx]);
    }

    public void SetActive(int idxNew) {
        for (int idx = 0; idx < mMarkerCount; idx++) {
            if (mMarkerActive[idx]) {
                mMarkerActive[idx] = false;
            }
        }
        mMarkerActive[idxNew] = true;
    }

    public void SetActiveXy(double x, double y) {
        for (int idx = 0; idx < mMarkerCount; idx++) {
            if (mMarkerActive[idx]) {
                mMarkerX[idx] = x;
                mMarkerY[idx] = y;
            }
        }
    }
        
    Axis mXaxis;
    public void setXaxis( Axis ax ){ mXaxis=ax; }
    public Axis getXaxis()         { return(mXaxis); }
    
    Axis mYaxis;
    public void setYaxis( Axis ax ){ mYaxis=ax; }
    public Axis getYaxis()         { return(mYaxis); }
    
    public void SetAxis(Axis xA, Axis yA) {
        mXaxis = xA;
        mYaxis = yA;
    }

    public Markers() {
        mMarkerCount = 10;
        mMarkerX = new double[mMarkerCount];
        mMarkerY = new double[mMarkerCount];
        mMarkerActive = new boolean[mMarkerCount];
        mMarkerVisible = new boolean[mMarkerCount];

        // Default data
        for (int n = 0; n < mMarkerCount; n++) {
            mMarkerX[n] = 6.28 * n / 10;
            mMarkerY[n] = (1.0 / (n + 1));
            mMarkerActive[n] = false;
            mMarkerVisible[n] = true;
        }
        mMarkerVisible[3] = false;
        mMarkerActive[4] = true;
    }

    void doDrawing(Graphics g) {
        Graphics2D g2d = (Graphics2D) g;

        g2d.setColor(mColor);

        int px0, py0;
        for (int n = 0; n < mMarkerCount; n++) {
            if (!mMarkerVisible[n]) {
                continue;
            }
            px0 = mXaxis.LogicalToPhysical(mMarkerX[n]);
            py0 = mYaxis.LogicalToPhysical(mMarkerY[n]);
            Misc.DrawNumberSingle(g2d, px0, py0, 'd', 6, true, n);
        }
    }
} // End of Markers

