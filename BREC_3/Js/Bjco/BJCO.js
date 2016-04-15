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

// the following line tells jslint this will execute in a browser
// and that certain variable (e.g. document) are defined
/*jslint browser:true */

////////////////////////////////////////////////////////////////////////////////
//
// BREC Javascript Console
//
////////////////////////////////////////////////////////////////////////////////

var BJCO = BJCO || {};

// This is the object constructor and must be provided a document object
// which will server as the DOM parent for the text area boxes for input
// and output.  After this, a call back must be registers to get line input
BJCO.Console = function( aParentDiv, aNcol, aNrow )
{
   // Create an output text area
   this.mOut = document.createElement('textarea');
   this.mOut.setAttribute("rows",aNrow);
   this.mOut.setAttribute("cols",aNcol);
   this.mOut.readOnly = true;

   this.mSep = document.createElement('br');

   // Create an input text area
   this.mIn = document.createElement('textarea');
   this.mIn.setAttribute("rows",1);
   this.mIn.setAttribute("cols",aNcol);

   // Add the output and input to parent (in that order)
   aParentDiv.appendChild( this.mOut );
   aParentDiv.appendChild( this.mSep );
   aParentDiv.appendChild( this.mIn );

   // Clear callback
   this.mCallBackFunc = 0;
};

// This is an internal method used as the input callback for the input
// text area.  It will process the newline and invoke the callback
BJCO.Console.prototype.InputFunction = function( aConObj )
{
   // Get the line of input
   var lineStr = aConObj.mIn.value;

   // console.log( lineStr.charCodeAt(lineStr.length - 1 ) );

   // If the last character is a new line then process full line
   if( lineStr.charCodeAt(lineStr.length - 1) == 10 ){

       // Clear input line
       this.mIn.value = "";
  
       // Output the new input line
       this.Output( lineStr );

       // Strip the new line and trim leading and trailing whitespace
       lineStr = lineStr.replace(/\r|\n/g,'');
       lineStr = lineStr.trim();

       // Invoke the callback with new line of input 
       this.mCallBackFunc( lineStr );
   }
};

// This method registers a callback to be invoked when a new line is ready
BJCO.Console.prototype.Register = function( aCallBackFunc )
{
   this.mCallBackFunc = aCallBackFunc;
   var me = this;
   this.mIn.addEventListener("input",
      function(){
        me.InputFunction( me ); // this->mIn at this point
      }
   );

};

// This method outputs a string to the output text area
BJCO.Console.prototype.Output = function( aLine )
{
   // Copy the input to output
   this.mOut.value = this.mOut.value + aLine;

   // Make sure output is at bottom of scroll
   this.mOut.scrollTop = this.mOut.scrollHeight;
};

// This method clears the output history
BJCO.Console.prototype.Clear = function( )
{
   // Set output text to empty
   this.mOut.value = "";
};

