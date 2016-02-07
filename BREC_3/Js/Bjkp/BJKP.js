//
//
// This source code is available under the "Simplified BSD license".
//
// Copyright (c) 2015, J. Kleiner
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
// BREC Javascript Keypad
//
////////////////////////////////////////////////////////////////////////////////

var BJKP = BJKP || {}; 

////////////////////////////////////////////////////////////////////////////////
///        ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
BJKP.Keypad = function( aDialog )
{
   this.mDialog       = aDialog;
   this.mCallbackFunc = 0;
};

BJKP.Keypad.prototype.ClickFunction = function( aObj )
{
   // console.log("BJKP.Keypad.prototype.ClickFunction this=" + this );
   // console.log("  click obj= " + aObj );
   // console.log("  btn value= " + aObj.value );

   // Get the node with specific id to find text id for this keypad
   var elem = this.mDialog.querySelectorAll('#BJKP-value');
   // console.log("  elem = " + elem);
   // console.log("  elems= " + elem.length);

   // Get the text area's value
   var val  = elem[0].value;
   // console.log("  val  = " + val );

   // Ok closes dialog and invokes supplied callback
   if( aObj.value == "Ok" ){
      this.mDialog.close();
      this.mCallbackFunc( val );
   }

   // Cancel just closes dialog without action
   else if ( aObj.value == "Cancel" ){
      this.mDialog.close();
   }

   // Clear
   else if ( aObj.value == "C" ){
      elem[0].value = "";
   }

   // Backspace
   else if ( aObj.value == "B" ){
      elem[0].value = val.substring(0,val.length-1);
   }

   // All other buttons append their value
   else{
      elem[0].value = val + aObj.value;
   }
};

BJKP.Keypad.prototype.Show = function( aInitValue, aCallbackFunc )
{
   var idx;

   // Save the supplied callback
   this.mCallbackFunc = aCallbackFunc;

   // Generate the dialog box content
   this.mDialog.innerHTML = this.GenKeypadContent( aInitValue );

   // Find all of the buttons in the content
   var btns = this.mDialog.querySelectorAll('.BJKP-button');
   // console.log(btns);

   // Keep a reference to the this keypad object
   var kp = this;

   // Add button click event handler to all buttons
   for( idx=0; idx<btns.length; idx++){
      btns[idx].addEventListener( 
          'click',
          function() {
              kp.ClickFunction( this ); // this->button at this point
          }
      );
   }

   // Show the modal dialog
   this.mDialog.showModal();

};

// Internal routine
BJKP.Keypad.prototype.GenButtonContent = function( aValue )
{
   var content;
   content = "<button class=\"BJKP-button\" ";
   content = content +     "value=\"" + aValue + "\">" + aValue;
   content = content + "</button>";
   return content  ;
};

// Internal routine
BJKP.Keypad.prototype.GenKeypadContent = function( aInitVal )
{
   var content;

   content = "<table>";   // add border=1 for debug illustration

   content = content + "<tr align=center><td>";
   content = content + "<textarea spellcheck=false rows=1 id=\"BJKP-value\">" +
                         aInitVal + "</textarea>";
   content = content + "</td></tr>";

   // Start embedded numeric button table
   content = content + "<tr align=center><td>";

   content = content + "<table>";

   content = content + "<tr>";
   content = content + "<td>" + this.GenButtonContent("7") + "</td>";
   content = content + "<td>" + this.GenButtonContent("8") + "</td>";
   content = content + "<td>" + this.GenButtonContent("9") + "</td>";
   content = content + "<td>" + this.GenButtonContent("+") + "</td>";
   content = content + "</tr>";

   content = content + "<tr>";
   content = content + "<td>" + this.GenButtonContent("4") + "</td>";
   content = content + "<td>" + this.GenButtonContent("5") + "</td>";
   content = content + "<td>" + this.GenButtonContent("6") + "</td>";
   content = content + "<td>" + this.GenButtonContent("-") + "</td>";
   content = content + "</tr>";

   content = content + "<tr>";
   content = content + "<td>" + this.GenButtonContent("1") + "</td>";
   content = content + "<td>" + this.GenButtonContent("2") + "</td>";
   content = content + "<td>" + this.GenButtonContent("3") + "</td>";
   content = content + "<td>" + this.GenButtonContent("E") + "</td>";
   content = content + "</tr>";

   content = content + "<tr>";
   content = content + "<td>" + this.GenButtonContent("C") + "</td>";
   content = content + "<td>" + this.GenButtonContent("0") + "</td>";
   content = content + "<td>" + this.GenButtonContent(".") + "</td>";
   content = content + "<td>" + this.GenButtonContent("B") + "</td>";
   content = content + "</tr>";

   content = content + "</table>";

   // End embedded numeric button table
   content = content + "</td></tr>";

   // Row with Ok and Cancel at corners
   content = content + "<tr><td>";
   content = content + "<div style=\"float: left\">" 
                     +  this.GenButtonContent("Ok") 
                     + "</div>";

   content = content + "<div style=\"float: right\">" 
                     + this.GenButtonContent("Cancel") 
                     + "</div>";
   content = content + "</td></tr>";

   // End of main table
   content = content + "</table>";
   return content;
};

