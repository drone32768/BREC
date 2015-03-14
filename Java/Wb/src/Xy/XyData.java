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
 * This class contains a vector/array of XY double data along with and x and
 * y axis for plotting/coordinate transforming the data. Accessor methods
 * are provided to manipulate the xy pairs. The data may be drawn via the
 * doDrawing method.
 */
public class XyData implements Serializable{
    
    static final long serialVersionUID = 42L;

    private double[] xarray = null;
    public void setXarray( double [] ra ){
        xarray = ra;
    }
    public double [] getXarray(){
        return(xarray);
    }
      
    private double[] yarray = null;
    public void setYarray( double [] ra ){
        yarray = ra;
    }
    public double [] getYarray(){
        return(yarray);
    }
    
    int count;
    public void setCount(int n) {
        count = n;
        if (xarray == null || count >= xarray.length) {
            xarray = new double[count];
            yarray = new double[count];
        }
    }
    public int getCount() {
        return (count);
    }
    
    Color mColor;
    public void setColor(Color c) {
        mColor = c;
    }
    public Color getColor(){
        return(mColor);
    }
    
    private Axis mYaxis;
    private Axis mXaxis;
    public void setXaxis( Axis ax ){ mXaxis=ax; }
    public Axis getXaxis()         { return(mXaxis); }
    public void setYaxis( Axis ax ){ mYaxis=ax; }
    public Axis getYaxis()         { return(mYaxis); }
    
    public void SetAxis(Axis xA, Axis yA) {
        mXaxis = xA;
        mYaxis = yA;
    }

    public void SetXy(int n, double x, double y) {
        if (n < 0 || n > count) {
            return;
        }
        xarray[n] = x;
        yarray[n] = y;
    }

    public double GetX(int n) {
        return (xarray[n]);
    }

    public double GetY(int n) {
        return (yarray[n]);
    }

    public XyData() {
        // Default data 
        setCount(8192);
    }

    void doDrawing(Graphics g) {
        Graphics2D g2d = (Graphics2D) g;

        g2d.setColor(mColor);

        int n, px0, py0, px1, py1;
        for (n = 1; n < count; n++) {
            px0 = mXaxis.LogicalToPhysical(xarray[n - 1]);
            py0 = mYaxis.LogicalToPhysical(yarray[n - 1]);
            px1 = mXaxis.LogicalToPhysical(xarray[n]);
            py1 = mYaxis.LogicalToPhysical(yarray[n]);
            g2d.drawLine(px0, py0, px1, py1);
        }
    }
}
