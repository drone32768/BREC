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

import java.awt.FontMetrics;
import java.io.ByteArrayOutputStream;
import java.io.PrintStream;

import java.awt.Graphics2D;
import java.io.Serializable;

/**
 * This class collect various utility and common services used with the XY
 * display material.
 */
public class Misc implements Serializable{
    
    static final long serialVersionUID = 42L;

    ///////////////////////////////////////////////////////////////////////////
    /**
     * This method draws a string using the provided graphics context, physical
     * device anchor coordinates and anchor position information.
     *
     * @param g2d Graphics context to use for drawing
     * @param px Device X coordinate to place anchor at
     * @param py Device Y coordinate to place anchor at
     * @param ap Anchor Position: 12 = 12 O'clock position 3 = 3 O'clock
     * position 6 = 6 O'clock position 9 = 9 O'clock position
     * @param ma Mark anchor indicator
     * @param s String to draw
     * @return No value returned.
     */
    public static void DrawStr(Graphics2D g2d, int px, int py, int anchorPos, boolean marker, String str) {
        FontMetrics metrics = g2d.getFontMetrics();
        int fontHeight = metrics.getHeight();
        int textWidth = metrics.stringWidth(str);
        switch (anchorPos) {
            case 3:
                g2d.drawString(str, px - 4 - textWidth, py + (fontHeight / 3));
                break;
            case 6:
                g2d.drawString(str, px - (textWidth / 2), py - 4);
                break;
            case 9:
                g2d.drawString(str, px + 4, py + (fontHeight / 3));
                break;
            case 12:
                g2d.drawString(str, px - (textWidth / 2), py + fontHeight);
                break;
        }
        if (marker) {
            g2d.drawOval(px - 2, py - 2, 4, 4);
            g2d.drawLine(px - 2, py, px + 2, py);
            g2d.drawLine(px, py - 2, px, py + 2);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    /**
     * This method draws a single double number using the provided graphics
     * context, physical device anchor coordinates, formating and anchor
     * position information.
     *
     * @param g2d Graphics context to use for drawing
     * @param px Device X coordinate to place anchor at
     * @param py Device Y coordinate to place anchor at
     * @param fmt Limited formatting indicator: 'e' = 8 character scientific 'd'
     * = unlimited character decimal
     * @param ap Anchor Position: 12 = 12 O'clock position 3 = 3 O'clock
     * position 6 = 6 O'clock position 9 = 9 O'clock position
     * @param ma Mark anchor indicator
     * @param r Number value to draw
     * @return No value returned.
     */
    public static void DrawNumberSingle(Graphics2D g2d, int px, int py, char fmt, int anchorPos, boolean marker, double r) {
        String str;
        if( fmt=='g' ){
            str = GreekNumberStr(r);    
        }
        else{
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            PrintStream             ps = new PrintStream(baos);            

            switch (fmt) {
                case 'e':
                    ps.format("%+1.1e", r);
                    break;
                case 'E':
                    ps.format("%+1.3e", r);
                    break;
                case 'd':
                    ps.format("%d", (int) r);
                    break;
                case '6':
                    ps.format("%+6.6f", r);
                    break;
                case '3':
                    ps.format("%+6.3f", r);
                    break;
                case ',':
                    ps.format("%,d", (long)r);
                    break;
            }
            str = baos.toString();
            ps.close();
        }
        
        DrawStr(g2d, px, py, anchorPos, marker, str);
    }
    
    static String GreekNumberStr( double value ){
        String suffix;
        int    exp1,exp2;
        double man;
        
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        PrintStream ps = new PrintStream(baos);
        
        if (value != 0) {
            exp1 = (int) Math.log10(Math.abs(value));
            exp2 = (exp1 / 3) * 3;
            if (exp1 < 0) { exp2 = exp2 - 3; }
            man = value / Math.pow(10.0, exp2);
        } else {
            exp2 = 0;
            man  = 0;
        }
        
        switch (exp2) {
            case  12:suffix="P";break;
            case  9 :suffix="G";break;
            case  6 :suffix="M";break;
            case  3 :suffix="k";break;
            case  0 :suffix="";break;
            case  -3:suffix="m";break;
            case  -6:suffix="u";break;
            case  -9:suffix="n";break;
            case -12:suffix="p";break;
            default :suffix="*";break;
        }

        if( man<1 ){
           ps.format("%+1.2f%s",man,suffix);    
        }
        else if( man<10 ){
           ps.format("%+1.2f%s",man,suffix);
        }
        else if( man<100 ){
           ps.format("%+1.1f%s",man,suffix);
        }
        else{
           ps.format("%+1.0f%s",man,suffix);
        }
        String str = baos.toString();
        ps.close();
        
        return(str);
    }    
}
