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
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Insets;
import java.awt.Font;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.event.MouseEvent;
import java.awt.image.BufferedImage;
import javax.imageio.ImageIO;
import javax.swing.JPanel;
import java.io.File;
import java.io.PrintWriter;
import java.io.Serializable;

/**
 * This class is an xy display directed at instrumentation applications. The
 * display is defined by an X and Y axis, two sets of xy data (one a primary
 * display and the other a memory), a set of markers, the displayed values
 * of enabled markers, and a cursor.
 *
 */
public class XyDisplay
        extends JPanel
        implements Serializable, MouseListener, MouseMotionListener {

    static final long serialVersionUID = 42L;
    transient Font          mFont;
    transient XyDisplayMenu mXyDisplayMenu;
    
    ///////////////////////////////////////////////////////////////////////////	
    Axis    xAxis;
    public void setxAxis( Axis v ){
        xAxis = v;
    }
    public Axis getxAxis(){
        return(xAxis);
    }

    ///////////////////////////////////////////////////////////////////////////	    
    Axis    yAxis;
    public void setyAxis( Axis v ){
        yAxis = v;
    }
    public Axis getyAxis(){
        return(yAxis);
    }
    
    ///////////////////////////////////////////////////////////////////////////	    
    Grid    mGrid;
    public void setGrid( Grid v ){
        mGrid = v;
    }
    public Grid getGrid(){
        return(mGrid);
    }
    
    ///////////////////////////////////////////////////////////////////////////	    
    boolean mMemVisible[];
    /**
     * This method makes the memory xy data visible or not
     *
     * @param ms = true if show memory data, false if not show
     */
    public void setMemVisible(int mn, boolean ms) {
        mMemVisible[mn] = ms;
    }
    public boolean getMemVisible( int mn ){
        return(mMemVisible[mn]);
    }

    ///////////////////////////////////////////////////////////////////////////
    XyData  mXyMem[];
    public void setXyMem( int mn, XyData v ){
        mXyMem[mn] = v;
    }
    public XyData getXyMem( int mn ){
        return( mXyMem[mn] );
    }
    
    /**
     * This methods copies the current xy data to the memory xy
     */
    public void setMemCopyXy( int mn ) {
        mXyMem[mn].setCount(mXyData.getCount());
        for (int n = 0; n < mXyData.getCount(); n++) {
            mXyMem[mn].SetXy(n, mXyData.GetX(n), mXyData.GetY(n));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    private XyData  mXyEnvH;
    public void setXyEnvH( XyData xyd ){
        mXyEnvH = xyd;
    }
    public XyData getXyEnvH(){
        return(mXyEnvH);
    }
    
    private XyData  mXyEnvL;
    public void setXyEnvL( XyData xyd ){
        mXyEnvL = xyd;
    }
    public XyData getXyEnvL(){
        return(mXyEnvL);
    }
    
    private boolean mEnvShow;
    /**
     * This methods enables/disables showing of the envelope data
     * @param show 
     */
    public void setEnvVisible( boolean show ){
        mEnvShow = show;
    }
    /**
     * This method produces the current state of show/not show envelope data
     * @return 
     */
    public boolean getEnvVisible(){
        return(mEnvShow);
    }
    
    boolean mEnvActive;
    /**
     * This method sets the state of envelope active.  On transition from
     * off to on, the envelope is reset to alternate max/min
     * @param ac 
     */
    public void setEnvActive( boolean ac ){
        mEnvActive = ac;
        resetEnv();
    }
    
    public boolean getEnvActive(){
        return(mEnvActive);
    } 
    
    public void resetEnv(){
        for (int n = 0; n < mXyEnvH.getCount(); n++) {
            mXyEnvH.SetXy(n, mXyData.GetX(n), -Double.MAX_VALUE);
            mXyEnvL.SetXy(n, mXyData.GetX(n),  Double.MAX_VALUE);  
        }         
    }

    ///////////////////////////////////////////////////////////////////////////
    private Markers     mMarkers;
    public  void setMarkers(Markers ms ){
        mMarkers=ms;
    }
    public Markers getMarkers(){
        return(mMarkers);
    }
    
    private MarkerTable mMarkerTbl;
    public void setMarkerTable( MarkerTable mt ){
        mMarkerTbl=mt;
    }
    public MarkerTable getMarkerTable(){
        return( mMarkerTbl );
    }
    
    /**
     * This method returns the visible state of the specified marker
     *
     * @param idx marker index in question
     * @return visible state of marker
     */
    public boolean getMarkerVisible(int idx) {
        return (mMarkers.GetVisible(idx));
    }

    /**
     * This method makes the specified marker visible or not.
     *
     * @param idx = index of marker to modify
     * @param on = state of visibility
     */
    public void setMarkerVisible(int idx, boolean on) {
        mMarkers.SetVisible(idx, on);
    }

    /**
     * This methods makes the specified marker active
     *
     * @param idx
     *
     */
    public void setMarkerActive(int idx) {
        mMarkers.SetActive(idx);
    }
    
    public boolean getMarkerActive(int idx){
        return( mMarkers.GetActive(idx) );
    }

    public int getMarkerCount(){
        return( mMarkers.getMarkerCount());
    }
    
            
    /**
     * This method sets the specified marker xy
     * @param idx
     * @param x
     * @param y
     */
    public void setMarkerXy(int idx, double x, double y) {
        mMarkers.SetX(idx, x);
        mMarkers.SetY(idx, y);
    }
    
    /**
     * This method will update the active markers with the current cursor
     * location.
     */
    public void setMarkerActiveToCursor() {
        mMarkers.SetActiveXy(mCursor.GetX(), mCursor.GetY());
    }

    /**
     * This method makes all markers visible or not
     *
     * @param on = true show all markers, false=hide all
     */
    public void setMarkersVisible(boolean on) {
        for (int idx = 0; idx < mMarkers.getMarkerCount(); idx++) {
            mMarkers.SetVisible(idx, on);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    private Cursor  mCursor;
    public void   setXyCursor(Cursor c) { mCursor=c; }
    public Cursor getXyCursor() {return(mCursor); }
    
    /**
     * This method updates the cursor location with the provided physical x/y.
     * This location is translated into logical units given the current x/y axis
     * state and saved as the new current cursor location.
     *
     * @param px
     * @param py
     */
    public void setCursorXy(int px, int py) {
        mCursor.Update(px, py);
    }

    ///////////////////////////////////////////////////////////////////////////
    private Title   mTitle;
    public void setTitle( Title t ){
        mTitle = t;
    }
    public Title getTitle(){
        return(mTitle);
    }
    
    /**
     * This method sets the title string for the plot. NOTE: This is a single
     * line, prefixed with current date and time and centered above the graph.
     *
     * @param str
     */
    public void setTitleStr(String str) {
        mTitle.setTitleStr(str);
    }
    public String getTitleStr(){
        return(mTitle.getTitleStr());
    }

    /**
     * This method sets the caption string in the title area. NOTE: The caption
     * is a single line, centered below the title line.
     *
     * @param str
     */
    public void setCaptionStr(String str) {
        mTitle.setCaptionStr(str);
    }
    public String getCaptionStr(){
        return(mTitle.getCaptionStr());
    }

    /**
     * This method sets the report 1 string in the title area. NOTE: The report
     * is a single line, to the left or right of the plot center
     *
     * @param str
     */
    public void setRpt1Str(String str) {
        mTitle.setRpt1Str(str);
    }
    public String getRpt1Str(){
        return(mTitle.getRpt1Str());
    }
    
    /**
     * This method sets the report 2 string in the title area. NOTE: The report
     * is a single line, to the left or right of the plot center
     *
     * @param str
     */
    public void setRpt2Str(String str) {
        mTitle.setRpt2Str(str);
    }
    public String getRpt2Str(){
        return(mTitle.getRpt2Str());
    }
    
    ///////////////////////////////////////////////////////////////////////////
    private XyData  mXyData;
    public void setXyData( XyData xyd ){
        mXyData = xyd;
    }
    public XyData getXyData(){
        return(mXyData);
    }
    
    /**
     * This method sets the upper limit on the number of xy points stored
     * @param n 
     */
    public void setXyCapacity(int n) {
        mXyData.setCount(n);
    }
    
    /**
     * This method sets the current xy data to the values provided.  Only 
     * the provided number of points are used (or the set xy capacity) 
     * whichever is smaller.
     * @param x
     * @param y
     * @param npts 
     */
    public void setXyData(double [] x, double [] y, int npts ){
        int idx;
        
        mXyData.setCount(npts);
        for(idx=0;idx<npts;idx++){
            mXyData.SetXy(idx,x[idx],y[idx]);
        }
        
        if( mEnvActive ){

            // if the new data has a different number of elements
            // then we have to reset the number of elements in the envelope
            // and reset the min/max values.
            // NOTE: This does not prevent problems where the X ranges have
            // changed but the number of elements remains the same.  In 
            // such cases the caller must manually reset the envelope
            if( mXyEnvH.getCount()!=mXyData.getCount()) {
                mXyEnvH.setCount(mXyData.getCount());
                mXyEnvL.setCount(mXyData.getCount());
                resetEnv();
            }

            double minmax;
            for (int n = 0; n < mXyEnvH.getCount(); n++) {
                minmax = Math.max( mXyData.GetY(n), mXyEnvH.GetY(n) );
                mXyEnvH.SetXy(n, mXyData.GetX(n), minmax);

                minmax = Math.min( mXyData.GetY(n), mXyEnvL.GetY(n) );  
                mXyEnvL.SetXy(n, mXyData.GetX(n), minmax);  
            }
        }
        
        if( mPeakPick.getPeakCount()>0 ){
            
            // Find the peaks
            mPeakPick.PickPeaks(x, y, npts);
            
            // Transfer peaks to markers
            for( idx=0;idx<mPeakPick.getPeakCount();idx++ ){
               mMarkers.SetX(idx, mPeakPick.GetPeakX(idx));
               mMarkers.SetY(idx, mPeakPick.GetPeakY(idx));    
            }
        }
    } // End of setXyData()

    /**
     * This method sets the number of major tick marks (with labels) on the axis
     * @param major 
     */
    public void setXmajorTicks( int major ){
        xAxis.setMajorTicks(major);
    }
    
   /**
     * This method sets the number of major tick marks (with labels) on the axis
     * @param major 
     */
    public void setYmajorTicks( int major ){
        yAxis.setMajorTicks(major);
    }
    
    /**
     * This method returns the current number of major tick marks (with labels)
     * on the axis.
     */
    public int getYmajorTicks(){
        return( yAxis.getMajorTicks() );
    }
    
    /**
     * This method sets the low and high limits for the x axis to the provided
     * values.
     * @param x0
     * @param x1 
     */
    public void setXlimits(double x0, double x1) {
        xAxis.SetLogicalExtents(x0, x1);
    }

    /**
     * This method sets the low and high limits for the y axis to the provided
     * values.
     * @param y0
     * @param y1 
     */
    public void setYlimits(double y0, double y1) {
        yAxis.SetLogicalExtents(y0, y1);
    }
    
    /**
     * This method sets the label for the x axis to the provided string.
     * @param str 
     */
    public void setXlabel( String str ){
        xAxis.setLabelStr(str);
    }

    /**
     * This method sets the label for the y axis to the provided string.
     * @param str 
     */
    public void setYlabel( String str ){
        yAxis.setLabelStr(str);
    }
    
    /**
     * This method sets the x axis number format
     * @param fmtCode 
     */
    public void setXformat( char fmtCode ){
        xAxis.setNumberFormat( fmtCode );
        mMarkerTbl.setXnumberFmt( fmtCode );
    }
    
    /**
     * This method sets the y axis number format
     * @param fmtCode 
     */
    public void setYformat( char fmtCode ){
        yAxis.setNumberFormat( fmtCode );
        mMarkerTbl.setYnumberFmt( fmtCode );
    }
    
    private PeakPick mPeakPick;
    public void setPeakPick( PeakPick pc ){
        mPeakPick = pc;
    }
    public PeakPick getPeakPick(){
        return(mPeakPick);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    public XyDisplay() {
        xAxis = new Axis();
        xAxis.SetLogicalExtents(0.0, 6.28);
        xAxis.setColor(Color.black);

        yAxis = new Axis();
        yAxis.SetLogicalExtents(-1.0, 1.0);
        yAxis.setColor(Color.black);

        mGrid = new Grid();
        mGrid.setXaxis(xAxis);
        mGrid.setYaxis(yAxis);
        mGrid.setColor(Color.gray);
        
        mCursor = new Cursor();
        mCursor.SetAxis(xAxis, yAxis);
        mCursor.setColor(Color.black);

        mXyData = new XyData();
        mXyData.SetAxis(xAxis, yAxis);
        mXyData.setColor(Color.blue);

        mXyMem      = new XyData[4];
        mMemVisible = new boolean[4];
        for(int mn=0;mn<mXyMem.length; mn++){
            mXyMem[mn] = new XyData();
            
            mXyMem[mn].SetAxis(xAxis, yAxis);
            if( 0==(mn%4) ){
                mXyMem[mn].setColor(Color.ORANGE);
            }
            if( 1==(mn%4) ){
                mXyMem[mn].setColor(Color.MAGENTA);
            }
            if( 2==(mn%4) ){
                mXyMem[mn].setColor(Color.RED);
            }
            if( 3==(mn%4) ){
                mXyMem[mn].setColor(Color.BLACK);
            }
        }
        
        mXyEnvH = new XyData();
        mXyEnvH.SetAxis(xAxis, yAxis);
        mXyEnvH.setColor(Color.green);
        
        mXyEnvL = new XyData();
        mXyEnvL.SetAxis(xAxis, yAxis);
        mXyEnvL.setColor(Color.green);
        
        mMarkers = new Markers();
        mMarkers.SetAxis(xAxis, yAxis);
        mMarkers.setColor(Color.red);

        mMarkerTbl = new MarkerTable();
        mMarkerTbl.setMarkers(mMarkers);
        mMarkerTbl.setColor(Color.blue);

        mTitle = new Title();
        mTitle.setColor(Color.black);

        mEnvShow = true;

        mFont          = new java.awt.Font("Courier New", 0, 12);
        
        mXyDisplayMenu = new XyDisplayMenu();
        mXyDisplayMenu.SetXyDisplay( this );
        

        mPeakPick = new PeakPick();
        mPeakPick.setPeakCount(0);

        addMouseListener(this);
        addMouseMotionListener(this);

        // Default data
        int n;
        double x;
        double y;
        mXyData.setCount(1024);
        mXyMem[0].setCount(mXyData.getCount());
        mXyEnvH.setCount(mXyData.getCount());
        mXyEnvL.setCount(mXyData.getCount());
        for (n = 0; n < 1024; n++) {
            x = 6.28 * n / 1024;
            y = Math.sin(x);
            mXyData.SetXy(n, x, 0.8*y);
            mXyMem[0].SetXy(n, x, 0.5*y);
            mXyEnvH.SetXy(n, x, 1.1*y);
            mXyEnvL.SetXy(n, x, 0.9*y);
        }
        mTitle.setTitleStr("Example-Title");
        mTitle.setCaptionStr("Example-Caption");
    }
    
    ///////////////////////////////////////////////////////////////////////////
    private void doDrawing(Graphics g) {

        Graphics2D g2d = (Graphics2D) g;
        Dimension size = getSize();
        Insets insets = getInsets();

        // Set the font for all text 
        g2d.setFont(mFont);

        // Figure out the extents of drawing area
        int w = size.width - insets.left - insets.right;
        int h = size.height - insets.top - insets.bottom;

        // Set and draw the X axis
        int px0, px1, py0, py1;
        px0 = (int) (0.1 * w);
        px1 = (int) (0.9 * w);
        py0 = (int) (0.9 * h);
        py1 = py0;
        xAxis.SetDrawingExtents(true, px0, py0, px1, py1);
        xAxis.doDrawing(g2d);

        // Set and draw the Y axis
        px0 = (int) (0.1 * w);
        px1 = px0;
        py0 = (int) (0.9 * h);
        py1 = (int) (0.1 * h);
        yAxis.SetDrawingExtents(false, px0, py0, px1, py1);
        yAxis.doDrawing(g2d);

        // Draw the grid
        mGrid.doDrawing( g2d );
        
        // Draw the memory/history data
        for(int mn=0;mn<mXyMem.length; mn++){
            if( mMemVisible[mn] ){
                mXyMem[mn].doDrawing(g2d);
            }
        }
        
        // Draw the envelope data
        if(mEnvShow){
            mXyEnvH.doDrawing(g2d);
            mXyEnvL.doDrawing(g2d);
        }

        // Draw the current xy data
        mXyData.doDrawing(g2d);

        // Draw the markers
        mMarkers.doDrawing(g);

        // Draw the marker table
        px0 = (int) (0.9 * w);
        px1 = w;
        py0 = (int) (0.1 * h);
        py1 = (int) (0.9 * h);
        mMarkerTbl.SetDrawingExtents(px0, py0, px1, py1);
        mMarkerTbl.doDrawing(g);

        // Draw the title
        mTitle.SetDrawingExtents(0, 0, w, (int) (0.1 * h));
        mTitle.doDrawing(g);

        // Draw the cursor
        px0 = (int) (0.9 * w);
        px1 = w;
        py0 = (int) (0.9 * h);
        py1 = h;
        mCursor.SetDrawingExtents(px0, py0, px1, py1);
        mCursor.doDrawing(g);

    }

    ///////////////////////////////////////////////////////////////////////////
    @Override
    public void paintComponent(Graphics g) {
        super.paintComponent(g);
        doDrawing(g);
    }

    ///////////////////////////////////////////////////////////////////////////
    public void mousePressed(MouseEvent e) {
        //saySomething("Mouse pressed; # of clicks: "
        //             + e.getClickCount(), e);
        maybeShowPopup(e);
    }

    public void mouseReleased(MouseEvent e) {
        // saySomething("Mouse released; # of clicks: "
        //             + e.getClickCount(), e);
        maybeShowPopup(e);

        if (1 == e.getButton()) {
            setMarkerActiveToCursor();
            repaint();
        }
    }

    public void mouseEntered(MouseEvent e) {
        // saySomething("Mouse entered", e);
    }

    public void mouseExited(MouseEvent e) {
        // saySomething("Mouse exited", e);
    }

    public void mouseClicked(MouseEvent e) {
        //saySomething("Mouse clicked (# of clicks: "
        //             + e.getClickCount() + ")", e);
    }

    public void mouseMoved(MouseEvent e) {
        // saySomething("Mouse moved", e);
        setCursorXy(e.getX(), e.getY());
        repaint();
    }

    public void mouseDragged(MouseEvent e) {
        // saySomething("Mouse dragged", e);
    }

    private void maybeShowPopup(MouseEvent e) {
        if (e.isPopupTrigger()) {
            mXyDisplayMenu.GetPopup().show(
                        e.getComponent(),
                        e.getX(), 
                        e.getY()
                    );
        }
    }

    void saySomething(String eventDescription, MouseEvent e) {
        System.out.println(eventDescription + " detected on "
                + e.getComponent().getClass().getName()
                + " " + e
                + ".");
    }
    
    /**
     * This method exports a PNG file version of the current xy plot
     */
    public void ExportPng( File file ) {
        BufferedImage bi = new BufferedImage(
                this.getSize().width,
                this.getSize().height,
                BufferedImage.TYPE_INT_ARGB);
        Graphics g = bi.createGraphics();
        this.paint(g);  //this == JComponent
        g.dispose();

        try {
            ImageIO.write(bi, "png", file);
        } catch (Exception e) {
        }
    }

    /**
     * This method exports a CSV file version of the current xy plot
     */
    public void ExportCsv( File file ) {
        
        try {
            int rows, idx;

            PrintWriter out = new PrintWriter(file);

            // Figure out the maximum number of rows we will have
            // This is the longest of any of the data sets
            rows = mXyData.getCount();
            for(int mn=0;mn<mXyMem.length; mn++){
                rows = Math.max(rows, mXyMem[mn].getCount() );
            }
            
            // Produce the header line
            out.println("X,Y,MemX,MemY,MarkerX,MarkerY," + mTitle.getCaptionStr());
            
            // Loop over the rows emitting one at a time
            for (idx = 0; idx < rows; idx++) {
                
                // single xy data value
                if (idx < mXyData.getCount() ) {
                    out.print(mXyData.GetX(idx) + "," + mXyData.GetY(idx));
                } else {
                    out.print("0,0");
                }
                
                // memories
                for(int mn=0;mn<mXyMem.length; mn++){
                    if (idx < mXyMem[mn].getCount() ) {
                        out.print("," + mXyMem[mn].GetX(idx) + "," + mXyMem[mn].GetY(idx));
                    } else {
                        out.print(",0,0");
                    }
                }
                
                // markers
                if (idx < 10) {
                    out.print("," + mMarkers.GetX(idx) + "," + mMarkers.GetY(idx));
                }
                out.println("");
                
            }// End of loop over output rows
            
            // Close file on completion
            out.close();

        } catch (Exception e) {
        }
    }
    
    public void SetPeakPickCount( int val ){
        // Disable any markers taken by existing peak count
        for( int idx=0;idx<mPeakPick.getPeakCount();idx++ ){
               mMarkers.SetVisible(idx, false);  
        }
        
        // Set the peak picker count
        mPeakPick.setPeakCount(val);
        
        // Enable a marker for each enabled peak
        for( int idx=0;idx<mPeakPick.getPeakCount();idx++ ){
               mMarkers.SetVisible(idx, true);  
        }
        repaint();
    }
    
    public int  GetPeakPickCount(){
        return(mPeakPick.getPeakCount());
    }
    
    public void setPeakFrac( double f ){
        mPeakPick.setPeakFrac(f);
    }
    
    public double getPeakFrac(){
        return( mPeakPick.getPeakFrac() );
    }
}
