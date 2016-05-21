//
//
// This source code is available under the "Simplified BSD license".
//
// Copyright (c) 2016, J. Kleiner
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, 
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright 
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the original author nor the names of its contributors 
//    may be used to endorse or promote products derived from this software 
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
// OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
"use strict";

////////////////////////////////////////////////////////////////////////////////
//
// BREC Javascript Graph Tool
//
////////////////////////////////////////////////////////////////////////////////

var BJGT = BJGT || {}; 

////////////////////////////////////////////////////////////////////////////////
/// Tran1D ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// This is a simple 1d transform.
//
BJGT.Tran1D = function( )
{
   this.mLmin = 0;
   this.mLmax = 10;

   this.mPmin = 0;
   this.mPmax = 50;
};

BJGT.Tran1D.prototype.SetLogicalLimits = function( aMin, aMax )
{
   this.mLmin = aMin;
   this.mLmax = aMax;
};

BJGT.Tran1D.prototype.SetPhysicalLimits = function( aDrawMin, aDrawMax )
{
   this.mPmin = aDrawMin;
   this.mPmax = aDrawMax;
};

BJGT.Tran1D.prototype.LogicalToPhysical = function( lVal )
{
   var pVal;

   pVal  = this.mPmin + 
           ( (this.mPmax - this.mPmin) * 
             (lVal - this.mLmin) / (this.mLmax - this.mLmin) );

   // console.log("LtoP: " + lVal + " : " + pVal);
   return pVal ;
};

BJGT.Tran1D.prototype.PhysicalToLogical = function( pVal )
{
   var lVal = this.mLmin + 
          (this.mLmax - this.mLmin) * 
            (pVal - this.mPmin) / (this.mPmax - this.mPmin);
   // console.log("PtoL: " + pVal + " : " + lVal);
   return lVal ;
};

////////////////////////////////////////////////////////////////////////////////
/// Tran2D ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// This is a simple 2d transform using an x and y 1d transform.
//
BJGT.Tran2D = function( )
{
   this.mXtrans = new BJGT.Tran1D();
   this.mYtrans = new BJGT.Tran1D();
};

BJGT.Tran2D.prototype.SetPhysicalLimits = function(aXmin, aXmax, aYmin, aYmax )
{
   this.mXtrans.SetPhysicalLimits( aXmin, aXmax );
   this.mYtrans.SetPhysicalLimits( aYmin, aYmax );
};

BJGT.Tran2D.prototype.SetLogicalLimits = function( aXmin, aXmax, aYmin, aYmax )
{
   this.mXtrans.SetLogicalLimits( aXmin, aXmax );
   this.mYtrans.SetLogicalLimits( aYmin, aYmax );
};

BJGT.Tran2D.prototype.XLtoP = function( lVal )
{
   return( this.mXtrans.LogicalToPhysical( lVal ) );
};

BJGT.Tran2D.prototype.XPtoL = function( pVal )
{
   return( this.mXtrans.PhysicalToLogical( pVal ) );
};

BJGT.Tran2D.prototype.YLtoP = function( lVal )
{
   return( this.mYtrans.LogicalToPhysical( lVal ) );
};

BJGT.Tran2D.prototype.YPtoL = function( pVal )
{
   return( this.mYtrans.PhysicalToLogical( pVal ) );
};

////////////////////////////////////////////////////////////////////////////////
/// Label //////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// The BJGT.Label is a text string to be displayed on an html5 canvas object.
// Once it is set, its width and height(approx) can be obtained.  It
// must be anchored (location and orientation from that location) and then
// it can be drawn.
//
// Width and Height are valid once the label is set.
// There is no Layout method
// SetAnchor must be applied prior to Draw()
//
BJGT.Label = function ( aCanvas )
{
   this.mCanvas  = aCanvas;
   this.mCtx     = this.mCanvas.getContext("2d");
   this.mTextStr = "textStr";
   this.mHeight  = 5;
   this.mWidth   = 5;
   this.mAnchor  = 12;
   this.mX0      = 0;
   this.mY0      = 0;
};

BJGT.Label.prototype.SetLabel = function ( aStr )
{
   this.mTextStr = aStr;
   this.mWidth   = this.mCtx.measureText(this.mTextStr).width;
   this.mHeight  = this.mCtx.measureText("M").width; // aprox...
};

BJGT.Label.prototype.Width= function()
{
   return(this.mWidth);
};

BJGT.Label.prototype.Height= function()
{
   return(this.mHeight);
};

// the attachement point of the label is a clock position on the label
BJGT.Label.prototype.SetAnchor = function( aClock, aX, aY )
{
   this.mAnchor = aClock;
   this.mX0     = aX;
   this.mY0     = aY;
};

BJGT.Label.prototype.Draw = function ()
{
   var x,y;

   switch( this.mAnchor ){
      case 12:
        x = this.mX0 - (this.mWidth/2);
        y = this.mY0 + this.mHeight;
        break;
      case 3:
        x = this.mX0 - this.mWidth;
        y = this.mY0 + (this.mHeight/2);
        break;
      case 6:
        x = this.mX0 - this.mWidth/2;
        y = this.mY0;
        break;
      case 9:
        x = this.mX0;
        y = this.mY0 + (this.mHeight/2);
        break;
   }
   this.mCtx.fillText(this.mTextStr,x,y);
};

////////////////////////////////////////////////////////////////////////////////
/// XyData /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// XyData is a container for a x and y vector of numbers.
// Sets of the data takes a copy of the provided data and holds it locally.
// Gets of the data produce references to the internal vectors.
// No layout is required prior to drawing.
//
BJGT.XyData = function ( aCanvas, a2Dtran )
{
   this.mCanvas   = aCanvas;
   this.mCtx      = this.mCanvas.getContext("2d");
   this.m2Dtran   = a2Dtran;

   this.mXvec      = [];
   this.mYvec      = [];
};

BJGT.XyData.prototype.Draw = function()
{
   var idx;
   var x0,y0;

   // TODO - get limits from transform and clamp any drawing to them

   x0 = this.m2Dtran.XLtoP( this.mXvec[0] );
   y0 = this.m2Dtran.YLtoP( this.mYvec[0] );
   this.mCtx.beginPath();
   this.mCtx.moveTo( x0, y0 );
   for(idx=1; idx<this.mXvec.length; idx++ ){
       x0 = this.m2Dtran.XLtoP( this.mXvec[idx] );
       y0 = this.m2Dtran.YLtoP( this.mYvec[idx] );
       this.mCtx.lineTo( x0, y0 );
   }
   this.mCtx.stroke();
};

BJGT.XyData.prototype.SetXy = function( xvec, yvec )
{
   var idx;
   for(idx=0;idx<xvec.length;idx++){
      this.mXvec[idx]=xvec[idx];
      this.mYvec[idx]=yvec[idx];
      // console.log(idx+":"+xvec[idx]+","+yvec[idx]);
   }
   this.mXvec.length = xvec.length;
   this.mYvec.length = yvec.length;
};

BJGT.XyData.prototype.GetXvec = function()
{
   return(this.mXvec);
};

BJGT.XyData.prototype.GetYvec = function()
{
   return(this.mYvec);
};

BJGT.XyData.prototype.SetXyValue = function( aIdx, aX, aY)
{
   this.mXvec[aIdx] = aX;
   this.mYvec[aIdx] = aY;
};

BJGT.XyData.prototype.GetYValue = function( aIdx )
{
   return( this.mYvec[ aIdx ] );
}; 

BJGT.XyData.prototype.SetYValue = function( aIdx, aY )
{
   this.mYvec[ aIdx ] = aY;
}; 

BJGT.XyData.prototype.SetLength = function ( len )
{
   this.mXvec.length = len;
   this.mYvec.length = len;
};

////////////////////////////////////////////////////////////////////////////////
/// Markers ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// Markers are displayed on an html5 canvas.  A 2D transform (logical/physical)
// must be provided along with the canvas.
//
// A table of 10 markers is displayed at the right edge of the canvas.
// The markers are drawn on the plotting area using the 2D transform provided
// as symbols with their number in the upper right of them.
//
// Layout must only be invoked if the size of the canvas changes.
// Width and Height are only valid after layout
//
// The marker table can be displayed in absolute or relative mode.  In 
// absolute, all x,y values are the absolute values of the marker.  In 
// relative mode, the table values displayed are relative to marker 0 and
// marker 0 is displayed as absolute.
//
// A marker can be in 3 states: pin, focus, auto.  Pin means a value has
// been set and should be retained, focus means it can accept a location
// from a pick or other focused user operation, and auto means it is available
// for autonomous use by other software
//
BJGT.Markers = function ( aCanvas, a2Dtran )
{
   var idx;

   this.mCanvas     = aCanvas;
   this.mCtx        = this.mCanvas.getContext("2d");
   this.m2Dtran     = a2Dtran;
   this.Visible     = 1;
   this.mMaxMarkers = 10;
   this.mMarker     = [];
   this.mWidth      = 10;
   this.mHeight     = 10;
   this.mRel        = 0;
 
   // State constants
   this.STATE_AUTO  = 1; // available for analysis purposes
   this.STATE_FOCUS = 2; // available for clicks
   this.STATE_PIN   = 3; // has been set and should be retained

   this.mTblHdr = new BJGT.Label( this.mCanvas );
   this.mTblHdr.SetLabel("NA");

   for(idx = 0; idx < this.mMaxMarkers; idx++ ) {
      this.mMarker[idx] = {};
      this.mMarker[idx].mX = 0;
      this.mMarker[idx].mY = 0;

      this.mMarker[idx].mIdLbl = new BJGT.Label( this.mCanvas );
      this.mMarker[idx].mIdLbl.SetLabel("0");

      this.mMarker[idx].mXvLbl = new BJGT.Label( this.mCanvas );
      this.mMarker[idx].mXvLbl.SetLabel("012345678901");

      this.mMarker[idx].mYvLbl = new BJGT.Label( this.mCanvas );
      this.mMarker[idx].mYvLbl.SetLabel("012345678901");

      this.mMarker[idx].mState = this.STATE_AUTO; 
   }

   // Make sure at lease one marker has focus
   this.mMarker[ this.mMaxMarkers -1 ].mState = this.STATE_FOCUS;
};

BJGT.Markers.prototype.SetShowRel = function( isrel )
{
   this.mRel = isrel;
};

BJGT.Markers.prototype.GetShowRel = function( )
{
   return( this.mRel );
};

BJGT.Markers.prototype.SetVisible = function( isv )
{
   this.Visible = isv;
};

BJGT.Markers.prototype.GetVisible = function( )
{
   return( this.Visible );
};

BJGT.Markers.prototype.PinMarker = function( aIdx )
{
   this.mMarker[aIdx].mState = this.STATE_PIN;
}

BJGT.Markers.prototype.FocusMarker = function( aIdx )
{
   this.mMarker[aIdx].mState = this.STATE_FOCUS;
}

BJGT.Markers.prototype.AutoMarker = function( aIdx )
{
   this.mMarker[aIdx].mState = this.STATE_AUTO;
}

BJGT.Markers.prototype.GetNextFocusMarker = function( aIdx )
{
   var idx;

   for(idx = aIdx; idx < this.mMaxMarkers; idx++ ) {
      if( this.mMarker[idx].mState == this.STATE_FOCUS ){
          return( idx );
      }
   }
   return( this.mMaxMarkers - 1 ); // always return valid index
}

BJGT.Markers.prototype.GetNextAutoMarker = function( aIdx )
{
   var idx;

   for(idx = aIdx; idx < this.mMaxMarkers; idx++ ) {
      if( this.mMarker[idx].mState == this.STATE_AUTO ){
          return( idx );
      }
   }
   return( this.mMaxMarkers - 1 ); // always return valid index
}

BJGT.Markers.prototype.Layout = function()
{
   var idx,y0,yd;
   var xc;
   var ystart;

   xc     = this.mMarker[0].mXvLbl.Width() / 2;
   y0     = this.mMarker[0].mXvLbl.Height();
   yd     = this.mMarker[0].mXvLbl.Height() + 1;
   ystart = 0;

   this.mTblHdr.SetAnchor(3, this.mCanvas.width-1 - xc,y0);
   y0+=yd;

   for(idx=0;idx<this.mMaxMarkers;idx++){

      this.mMarker[idx].mIdLbl.SetAnchor(3, this.mCanvas.width-1 - xc, y0 );
      y0+=yd;

      this.mMarker[idx].mXvLbl.SetAnchor(3, this.mCanvas.width-1, y0 );
      y0+=yd;

      this.mMarker[idx].mYvLbl.SetAnchor(3, this.mCanvas.width-1, y0 );
      y0+=yd;

      y0+=3; // Small vertical seperation 
   }

   this.mHeight = y0 - ystart;
   this.mWidth  = this.mMarker[0].mXvLbl.Width() + 1;
};

BJGT.Markers.prototype.Width = function() {
   return( this.mWidth );
};

BJGT.Markers.prototype.Height = function() {
   return( this.mHeight );
};

BJGT.Markers.prototype.PlaceMarker = function( aIdx, aX, aY )
{
   this.mMarker[aIdx].mX = aX;
   this.mMarker[aIdx].mY = aY;
};

BJGT.Markers.prototype.Draw = function()
{
   var idx;
   var px,py;
   var delp;
   var xv,yv;

   if( !this.Visible ) { 
      return;
   }

   // TODO: formatting of table is not always correct based on neg expo

   // Draw the table header
   if( this.mRel ){
      this.mTblHdr.SetLabel("Rel");
   }
   else{
      this.mTblHdr.SetLabel("Abs");
   }
   this.mTblHdr.Draw();

   // Draw the table values
   for(idx=0;idx<this.mMaxMarkers;idx++){
      if( !this.mRel || 0==idx ){ // 0 is always abs
          xv = this.mMarker[idx].mX;
          yv = this.mMarker[idx].mY;
      }
      else{
          xv = this.mMarker[idx].mX - this.mMarker[0].mX;
          yv = this.mMarker[idx].mY - this.mMarker[0].mY;
      }
 
      this.mMarker[idx].mIdLbl.SetLabel( idx );
      this.mMarker[idx].mIdLbl.Draw();

      this.mMarker[idx].mXvLbl.SetLabel(xv.toExponential(5));
      this.mMarker[idx].mXvLbl.Draw();

      this.mMarker[idx].mYvLbl.SetLabel(yv.toExponential(5));
      this.mMarker[idx].mYvLbl.Draw();
   }

   // Draw the individual makers
   delp = 4;
   for(idx=0;idx<this.mMaxMarkers;idx++){
      px = this.m2Dtran.XLtoP( this.mMarker[idx].mX );
      py = this.m2Dtran.YLtoP( this.mMarker[idx].mY );
      this.mCtx.fillText( idx, px+delp, py-delp );

      this.mCtx.beginPath();
      this.mCtx.moveTo( px,      py-delp );
      this.mCtx.lineTo( px-delp, py+delp );
      this.mCtx.lineTo( px+delp, py+delp );
      this.mCtx.lineTo( px,      py-delp );
      this.mCtx.stroke();
   }
};

////////////////////////////////////////////////////////////////////////////////
/// Peak ///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// Peakpicking selects a number of peaks from a given xy dataset.
// A peak is a local maxima where local is +/- 5% of the total number of samples
//
BJGT.Peak = function( )
{
   this.mMask = [];
};

BJGT.Peak.prototype.AssignMarkers = function(xvec,yvec,aMarkers, aMarkerCount )
{
   var idx;
   var markerIdx;
   var lmax;
   var loIdx,hiIdx,miIdx,dIdx;
   var peakCount;

   // console.log("AssignMarkers: peaks picked="+aMarkerCount);

   // Clear the mask
   for(idx=0;idx<xvec.length;idx++){
      this.mMask[idx]=1;
   }

   // Establish peak mask as fraction of x points
   dIdx = Math.round( 0.05 * xvec.length );

   // Start looking for auto markers at 0
   markerIdx = 0;

   // Loop over looking for peaks
   for(peakCount=0;peakCount<aMarkerCount;peakCount++){

       // Find index of current non-masked peak
       lmax = -Number.MAX_VALUE;
       miIdx=0;
       for(idx=0;idx<xvec.length;idx++){
          if( (yvec[idx] > lmax) && this.mMask[idx] ){ 
             lmax=yvec[idx];
             miIdx=idx;
          }
       }

       // Place the next auto marker at this peak
       markerIdx = aMarkers.GetNextAutoMarker(markerIdx);
       aMarkers.PlaceMarker( markerIdx, xvec[miIdx], yvec[miIdx] );
       markerIdx++;

       // Establish low end of mask update
       loIdx = miIdx - dIdx;
       if(loIdx<0){
         loIdx=0;
       }
    
       // Establish high end of mask update
       hiIdx = miIdx + dIdx;
       if(hiIdx>xvec.length){
          hiIdx=(xvec.length-1);
       }
    
       // console.log("AssignMarkers: peak at "+xvec[miIdx]);
       // console.log("AssignMarkers: masking idx "+
       //                         loIdx+" to "+hiIdx + " del="+dIdx);

       // Apply the mask for this peak
       for(idx=loIdx;idx<hiIdx;idx++){
          this.mMask[idx] = 0;
       }

   } // End of loop looking for peaks;

};

////////////////////////////////////////////////////////////////////////////////
/// Env  ///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
BJGT.Envelope = function( aCanvas, a2Dtran )
{
   this.mCanvas    = aCanvas;
   this.m2Dtran    = a2Dtran;
   this.mXyMax     = new BJGT.XyData( this.mCanvas, this.m2Dtran );
   this.mXyMin     = new BJGT.XyData( this.mCanvas, this.m2Dtran );

   this.mCanvas    = aCanvas;
   this.mCtx       = this.mCanvas.getContext("2d");
   this.m2Dtran    = a2Dtran;
   this.mReset     = 1;
};

BJGT.Envelope.prototype.Reset = function()
{
   this.mReset = 1;
};

BJGT.Envelope.prototype.Update = function(xvec,yvec)
{
   var idx;

   if( this.mReset ){
      console.log("Resetting envelope");
      for(idx=0;idx<xvec.length;idx++){
         this.mXyMax.SetXyValue( idx, xvec[idx], yvec[idx] ); 
         this.mXyMin.SetXyValue( idx, xvec[idx], yvec[idx] ); 
      }
      this.mXyMin.SetLength( xvec.length );
      this.mXyMax.SetLength( xvec.length );
      this.mReset = 0;
   }
 
   else{
      for(idx=0;idx<xvec.length;idx++){
         if( yvec[idx] > this.mXyMax.GetYValue(idx) ){
             this.mXyMax.SetYValue( idx, yvec[idx] ); 
         }
         if( yvec[idx] < this.mXyMin.GetYValue(idx) ){
             this.mXyMin.SetYValue( idx, yvec[idx] ); 
         }
      }
   }
};

BJGT.Envelope.prototype.Draw = function()
{
   this.mXyMax.Draw();
   this.mXyMin.Draw();
};

////////////////////////////////////////////////////////////////////////////////
/// Number formater ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
BJGT.Fmt = function()
{
   this.fmt = '3';
};

BJGT.Fmt.prototype.SetFmt = function ( fmt )
{
   this.fmt = fmt; 
};

BJGT.Fmt.prototype.GetNumber = function ( num )
{
   switch( this.fmt ){
      case '3':
         return( num.toFixed(3) );
      case 'i':
         return( num.toLocaleString() );
      default:
         return( num.toFixed(3) );
   }
};

////////////////////////////////////////////////////////////////////////////////
/// Reticle ///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// A reticle is a mesh of lines on the front of an instrument.  This
// varient includes axis values.
//
// No layout is necessary.  When the number of ticks or format of numbers
// is changed it must be redrawn to take effect.
//
BJGT.Reticle = function( aCanvas, a2Dtran )
{
   this.mCanvas= aCanvas;
   this.mCtx   = this.mCanvas.getContext("2d");
   this.m2Dtran= a2Dtran;

   this.mLabel = new  BJGT.Label( this.mCanvas );

   this.mXticks= 5;
   this.mYticks= 5;
  
   this.mXfmt  = new BJGT.Fmt();
   this.mYfmt  = new BJGT.Fmt();
};

BJGT.Reticle.prototype.SetXticks = function( val )
{
   this.mXticks = val;
};

BJGT.Reticle.prototype.SetYticks = function( val )
{
   this.mYticks = val;
};

BJGT.Reticle.prototype.SetXfmt = function( val )
{
   this.mXfmt.SetFmt(val);
};

BJGT.Reticle.prototype.SetYfmt = function( val )
{
   this.mYfmt.SetFmt(val);
};

BJGT.Reticle.prototype.Draw = function()
{
   var x0,x1,y0,y1;
   var xd,xl;
   var yd,yl;
   var idx;

   y0 = this.m2Dtran.YLtoP( this.m2Dtran.mYtrans.mLmin );
   y1 = this.m2Dtran.YLtoP( this.m2Dtran.mYtrans.mLmax  );
   xl = this.m2Dtran.mXtrans.mLmin;
   xd =  (this.m2Dtran.mXtrans.mLmax - this.m2Dtran.mXtrans.mLmin ) / 
         this.mXticks;
   for( idx=0; idx<=this.mXticks; idx++){
      x0 = this.m2Dtran.XLtoP( xl );
      this.mCtx.beginPath();
      this.mCtx.moveTo( x0, y0 );
      this.mCtx.lineTo( x0, y1 );
      this.mCtx.stroke();

      this.mLabel.SetLabel( this.mXfmt.GetNumber(xl) );
      if( 0==idx ){
          this.mLabel.SetAnchor(12,x0+this.mLabel.Width()/2,y0+1);
      }
      else if( this.mXticks==idx ){
          this.mLabel.SetAnchor(12,x0-this.mLabel.Width()/2,y0+1);
      }
      else{
          this.mLabel.SetAnchor(12,x0,y0+1);
      }

      this.mLabel.Draw();

      xl += xd;
   }

   x0 = this.m2Dtran.XLtoP( this.m2Dtran.mXtrans.mLmin );
   x1 = this.m2Dtran.XLtoP( this.m2Dtran.mXtrans.mLmax  );
   yl = this.m2Dtran.mYtrans.mLmin;
   yd =  (this.m2Dtran.mYtrans.mLmax - this.m2Dtran.mYtrans.mLmin ) / 
         this.mYticks;
   for( idx=0; idx<=this.mYticks; idx++){
      y0 = this.m2Dtran.YLtoP( yl );
      this.mCtx.beginPath();
      this.mCtx.moveTo( x0, y0 );
      this.mCtx.lineTo( x1, y0 );
      this.mCtx.stroke();

      this.mLabel.SetLabel( this.mYfmt.GetNumber(yl) );
      this.mLabel.SetAnchor(3,x0-1,y0);
      this.mLabel.Draw();

      yl += yd;
   }
};

////////////////////////////////////////////////////////////////////////////////
/// XyDisplay //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// XyDisplay is the main exported object and provides a reasonbly high
// performance graphing and display interface for 2D data.
//
// When parameters other than the xy data have changed Layout() must be invoked
// prior to drawing.
//
BJGT.XyDisplay = function( aCanvas )
{
   var idx;

   // Establish the canvas and context
   this.mCanvas     = aCanvas;
   this.mCtx        = this.mCanvas.getContext("2d");
   console.log( "BJGT.XyDisplay: "+this.mCanvas.width+"x"+this.mCanvas.height );

   // Establish a 2D transform between logical and physical coordinates
   // and create a default 1:1 translation to start with.
   this.m2Dtran     = new BJGT.Tran2D();
   this.m2Dtran.SetPhysicalLimits( 1, 
                                   this.mCanvas.width  - 1,
                                   this.mCanvas.height - 1, 
                                   1);

   // Establish a title, x, and y labels
   this.mLabelTitle = new BJGT.Label( this.mCanvas );
   this.mLabelY     = new BJGT.Label( this.mCanvas );
   this.mLabelX     = new BJGT.Label( this.mCanvas );
 
   // Establish a reticle, markers, and core xy data trace
   this.mReticle    = new BJGT.Reticle( this.mCanvas, this.m2Dtran );
   this.mMarkers    = new BJGT.Markers( this.mCanvas, this.m2Dtran );
   this.mXyData     = new BJGT.XyData( this.mCanvas, this.m2Dtran );

   // Establish a history of traces
   this.mXyTrace    = []; 
   for(idx=0;idx<4;idx++){
       this.mXyTrace[idx] = {};
       this.mXyTrace[idx].mVisible= 0;
       this.mXyTrace[idx].mXyData = new BJGT.XyData(this.mCanvas, this.m2Dtran);
   }
 
   // Establish an envelope history
   this.mEnvelope   = new BJGT.Envelope( this.mCanvas, this.m2Dtran );
   this.mEnvEnable  = 0;
   this.mEnvVisible = 0;

   // Establish a peak picker
   this.mPeakPicker = new BJGT.Peak();
   this.mPicCount   = 0;

   // Save off computed colors for later use
   this.mTextColor  = window.getComputedStyle( this.mCanvas, null ).
                               getPropertyValue("color");
};

BJGT.XyDisplay.prototype.Layout = function()
{
   var topMargin;
   var bottomMargin;
   var leftMargin;
   var rightMargin;

   // TODO - allow for non fixed fonts
   this.mCtx.font="10px Arial";

   // Markers sit at the right of the canvas
   this.mMarkers.Layout();
   rightMargin = this.mMarkers.Width();

   // Title sits at the top of the canvas
   this.mLabelTitle.SetAnchor(12, this.mCanvas.width/2, 0);
   topMargin   = this.mLabelTitle.Height() + 1;

   // X label sits at the bottom of the canvas
   this.mLabelX.SetAnchor(6,
         this.mLabelY.Width() + 
            (this.mCanvas.width-this.mLabelY.Width()-rightMargin)/2,
         this.mCanvas.height-1);
   bottomMargin= 2*(this.mLabelX.Height() + 1);

   // Y label sits at the left of the canvas
   this.mLabelY.SetAnchor(9,
         1, 
         topMargin + (this.mCanvas.height - topMargin - bottomMargin)/2);
   leftMargin  = this.mLabelY.Width() + 1;

   // TODO
   leftMargin = 2*leftMargin;

   // console.log("topMargin    = "+topMargin);
   // console.log("bottomMargin = "+bottomMargin);
   // console.log("leftMargin   = "+leftMargin);
   // console.log("rightMargin  = "+rightMargin);

   this.m2Dtran.SetPhysicalLimits( leftMargin, 
                                   this.mCanvas.width  - rightMargin,
                                   this.mCanvas.height - bottomMargin, 
                                   topMargin);

};

BJGT.XyDisplay.prototype.Draw = function()
{
   var idx;

   // TODO make colors CSS configurable

   // Clear the canvas
   this.mCtx.clearRect(0, 0, this.mCanvas.width, this.mCanvas.height);

   // Draw any visible histories
   for(idx=0;idx<this.mXyTrace.length;idx++){
       if( this.mXyTrace[idx].mVisible ){
          switch( idx ){
             case 0:
               this.mCtx.strokeStyle = "cyan";
               break;
             case 1:
               this.mCtx.strokeStyle = "orange";
               break;
             case 2:
               this.mCtx.strokeStyle = "magenta";
               break;
             default:
               this.mCtx.strokeStyle = "yellow";
               break;
          }
          this.mXyTrace[idx].mXyData.Draw();
       }
   }

   // Draw the envelope
   if( this.mEnvVisible ){
      this.mCtx.strokeStyle = "green";
      this.mEnvelope.Draw();
   }

   // Draw the core data
   this.mCtx.strokeStyle = "blue";
   this.mXyData.Draw();

   // Draw the reticle
   this.mCtx.strokeStyle = "grey";
   this.mReticle.Draw();

   // Draw the title and labels
   this.mCtx.fillStyle   = this.mTextColor;
   this.mLabelTitle.Draw();
   this.mLabelY.Draw();
   this.mLabelX.Draw();

   // Draw the markers
   this.mCtx.strokeStyle = "red";
   this.mMarkers.Draw();
};

BJGT.XyDisplay.prototype.EnableMouse = function()
{
   var thisBxy;

   // Create a copy of the real this for use in event handler
   thisBxy = this;

   this.mCanvas.addEventListener(
       'click', 
        function( evt ){
            // NOTE: this is the DOM object that initiated the event
            var midx;
            var lx,ly;

            var rect = thisBxy.mCanvas.getBoundingClientRect();

            lx=thisBxy.m2Dtran.XPtoL( evt.clientX - rect.left );
            ly=thisBxy.m2Dtran.YPtoL( evt.clientY - rect.top );

            // Find the first marker in focus and place it at click point
            midx = thisBxy.mMarkers.GetNextFocusMarker(0);
            thisBxy.mMarkers.PlaceMarker(midx,lx,ly);

            // thisBxy.PlaceMarker(9,lx,ly); // Mouse always uses marker 9

            thisBxy.Draw();
        },
        false );
};

BJGT.XyDisplay.prototype.SetTitle = function ( aTitle )
{
   this.mLabelTitle.SetLabel(aTitle);
};

BJGT.XyDisplay.prototype.SetYlabel = function ( aStr )
{
   this.mLabelY.SetLabel( aStr );
};

BJGT.XyDisplay.prototype.SetXlabel = function ( aStr )
{
   this.mLabelX.SetLabel( aStr );
};

BJGT.XyDisplay.prototype.MarkersVisible = function( isv )
{
   this.mMarkers.SetVisible( isv );
};

BJGT.XyDisplay.prototype.MarkersRelative = function( isrel )
{
   this.mMarkers.SetShowRel( isrel );
};

BJGT.XyDisplay.prototype.MarkersIsVisible = function( )
{
   return( this.mMarkers.GetVisible() );
};

BJGT.XyDisplay.prototype.SetMarker = function( aIdx, aX, aY )
{
   this.mMarkers.PinMarker( aIdx )
   this.mMarkers.PlaceMarker( aIdx, aX, aY );
};

BJGT.XyDisplay.prototype.PinMarker = function( aIdx )
{
   this.mMarkers.PinMarker( aIdx )
}

BJGT.XyDisplay.prototype.FocusMarker = function( aIdx )
{
   this.mMarkers.FocusMarker( aIdx );
};

BJGT.XyDisplay.prototype.AutoMarker = function( aIdx )
{
   this.mMarkers.AutoMarker( aIdx );
};

BJGT.XyDisplay.prototype.SetLogicalLimits = function (aXmin,aXmax,aYmin,aYmax)
{
   this.m2Dtran.SetLogicalLimits( aXmin, aXmax,
                                  aYmin, aYmax );
};

BJGT.XyDisplay.prototype.SetXy = function ( xvec, yvec )
{
   this.mXyData.SetXy( xvec, yvec );

   if( this.mEnvEnable ){
      this.mEnvelope.Update( xvec, yvec );
   }

   if( this.mPicCount>0 ){
      this.mPeakPicker.AssignMarkers( 
               xvec, yvec, this.mMarkers, this.mPicCount );
   }
};

BJGT.XyDisplay.prototype.PeakPick = function ( aCnt )
{
    this.mPicCount = aCnt;
};

BJGT.XyDisplay.prototype.MedianPick = function ( aCnt )
{
    this.mMedCount = aCnt;
};

BJGT.XyDisplay.prototype.TraceSave = function ( aBfn )
{
    this.mXyTrace[aBfn].mXyData.SetXy( this.mXyData.GetXvec(),
                                       this.mXyData.GetYvec() );
};

BJGT.XyDisplay.prototype.TraceVisible = function ( aBfn, aVis )
{
    this.mXyTrace[aBfn].mVisible=aVis;
};

BJGT.XyDisplay.prototype.TraceIsVisible = function ( aBfn )
{
    return( this.mXyTrace[aBfn].mVisible );
};

BJGT.XyDisplay.prototype.EnvIsVisible = function ()
{
   return( this.mEnvVisible );
};

BJGT.XyDisplay.prototype.EnvVisible = function ( aVis )
{
   this.mEnvVisible = aVis;
};

BJGT.XyDisplay.prototype.EnvIsEnable = function ()
{
   return( this.mEnvEnable );
};

BJGT.XyDisplay.prototype.EnvEnable = function ( aEnable )
{
   this.mEnvEnable=aEnable;
   this.mEnvelope.Reset();
};

BJGT.XyDisplay.prototype.SetXticks = function( val )
{
   this.mReticle.SetXticks( val );
};

BJGT.XyDisplay.prototype.SetYticks = function( val )
{
   this.mReticle.SetYticks( val );
};

BJGT.XyDisplay.prototype.SetXfmt = function( val )
{
   this.mReticle.SetXfmt( val );
};

BJGT.XyDisplay.prototype.SetYfmt = function( val )
{
   this.mReticle.SetYfmt( val );
};
