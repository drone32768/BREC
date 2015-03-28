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
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.io.Serializable;
import java.util.Date;
	
/**
 * This class encapsulates a title and caption that can be display on a
 * graph. The current date and time are automatically prepended to the
 * title. The title occupies the first line and the caption the second. Both
 * are centered within the drawing area specified.
 */
public class Title implements Serializable{
    
    static final long serialVersionUID = 42L;

    private String mCaptionStr;
    public void setCaptionStr(String s) {
        mCaptionStr = s;
    }
    public String getCaptionStr() {
        return (mCaptionStr);
    }
      
    private String mTitleStr;
    public void setTitleStr(String s) {
        mTitleStr = s;
    }
    public String getTitleStr() {
        return (mTitleStr);
    }
    
    private String mRpt1Str;
    public void setRpt1Str(String s) {
        mRpt1Str = s;
    }
    public String getRpt1Str() {
        return (mRpt1Str);
    }

    private String mRpt2Str;
    public void setRpt2Str(String s) {
        mRpt2Str = s;
    }
    public String getRpt2Str() {
        return (mRpt2Str);
    }
    
    private Color mColor;
    public void setColor(Color c) {
        mColor = c;
    }
    public Color getColor(){
        return(mColor);
    } 
    
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
    
    public Title() {
        // Default data
        mCaptionStr = new String("Default-Caption");
        mTitleStr   = new String("Default-Title");
        mRpt1Str    = new String("Report1");
        mRpt2Str    = new String("Report2");
    }

    void doDrawing(Graphics g) {
        Graphics2D g2d = (Graphics2D) g;
        int textWidth,px,py;

        g2d.setColor(mColor);
        String str;

        FontMetrics metrics = g2d.getFontMetrics();
        int fontHeight = metrics.getHeight();
        
        str = new Date().toString() + ":" + mTitleStr;
        textWidth = metrics.stringWidth(str);
        px = (mPx0 + (mPx1 - mPx0) / 2) - (textWidth / 2);
        // py = (mPy0 + (mPy1 - mPy0) / 2) - (fontHeight / 2);
        py = mPy0 + fontHeight;
        g2d.drawString(str, px, py);

        str = mCaptionStr;
        textWidth = metrics.stringWidth(str);
        px = (mPx0 + (mPx1 - mPx0) / 2) - (textWidth / 2);
        py = py + fontHeight;
        g2d.drawString(str, px, py);
        
        str = mRpt1Str;
        textWidth = metrics.stringWidth(str);
        px = (mPx0 + (mPx1 - mPx0) / 2) - (textWidth / 2);
        py = py + fontHeight;
        g2d.drawString(str, px, py);
        
        str = mRpt2Str;
        textWidth = metrics.stringWidth(str);
        px = (mPx0 + (mPx1 - mPx0) / 2) - (textWidth / 2);
        py = py + fontHeight;
        g2d.drawString(str, px, py);
    }
}
