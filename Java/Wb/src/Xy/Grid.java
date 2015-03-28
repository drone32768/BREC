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
    ////////////////////////////////////////////////////////////////////////
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.io.Serializable;

/**
 * The Axis class provides two basic capabilities: a) Drawing of an axis b)
 * Conversion from logical to physical coordinates.
 */
public class Grid implements Serializable{
    
    static final long serialVersionUID = 42L;
    
    private Axis mXaxis;
    public void setXaxis( Axis x ){
        mXaxis = x;
    }
    public Axis getXaxis(){
        return(mXaxis);
    }
    
    private Axis mYaxis;
    public void setYaxis( Axis y ){
        mYaxis = y;
    }
    public Axis getYaxis(){
        return(mYaxis);
    }

    Color mColor;
    public void setColor(Color c) {
        mColor = c;
    }
    public Color getColor(){
        return(mColor);
    }

    /**
     * No arg constructor.
     */
    public Grid() {
        // Default data
        setColor(Color.gray);
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

        int lnIdx, px1, py1, px2, py2;
        py1 = mYaxis.GetPhysicalMin();
        py2 = mYaxis.GetPhysicalMax();
        for(lnIdx=1;lnIdx<mXaxis.getMajorTicks();lnIdx++){
            px1 = mXaxis.GetPhysicalMin() + 
                    (     lnIdx *
                          (mXaxis.GetPhysicalMax()-mXaxis.GetPhysicalMin())/mXaxis.getMajorTicks()
                    );
            g2d.drawLine(px1, py1, px1, py2);    
        }
        px1 = mXaxis.GetPhysicalMin();
        px2 = mXaxis.GetPhysicalMax();
        for(lnIdx=1;lnIdx<mYaxis.getMajorTicks();lnIdx++){
            py1 = mYaxis.GetPhysicalMin() +
                    (      lnIdx *
                           (mYaxis.GetPhysicalMax()-mYaxis.GetPhysicalMin())/mYaxis.getMajorTicks()
                    );
            g2d.drawLine(px1, py1, px2, py1);         
        }


    }// End of doDrawing
} // End of Axis
