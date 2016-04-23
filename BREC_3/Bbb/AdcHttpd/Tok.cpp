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
#include <stdio.h>

#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#include "Tok.h"

////////////////////////////////////////////////////////////////////////////////
/**
 * This vector contains the set of registered callbacks for parsing
 * tokens.
 */
std::vector<TokCallback *> gvCallbacks;

////////////////////////////////////////////////////////////////////////////////
/**
 * This method registers a callback with the tokenizer.  The registered
 * callback will be invoked whenever tokens are available.  See
 * the description of the callback method in the header file.
 */
void
TokRegister( TokCallback *tokCb )
{
   gvCallbacks.push_back( tokCb );
}

////////////////////////////////////////////////////////////////////////////////
/**
 * This method is given a C string which is tokenized and returned.
 * NOTE: this is intended to be internal use only.
 */
std::vector<std::string>
TokParseCstr( const char *aInputCstr )
{
   int                       idx;
   std::string               str;
   bool                      inToken;
   char                      ch;
   std::vector<std::string>  tokens;

   // Initialize processing variables
   idx     = 0;
   inToken = false;
   tokens.clear();
   str.clear();

   // Loop while character available
   while( aInputCstr && aInputCstr[idx]!=0 ){

       // Get the next character
       ch = aInputCstr[idx];

       // Process this character
       if( ch==' ' || ch=='\t' ){
          if( inToken ){
             tokens.push_back(str);
             str.clear();
             inToken = false;
          }else{
             ; // do nothing, just consume the extra white space
          }
       }
       else{
          inToken = true;
          str.push_back( ch );
       }

       // Move to the next character no mater what
       idx++;
   }

   // If the last token was teriminated with 0 then add it to the vector
   if( inToken ){
      tokens.push_back(str);
   }

   // Return a copy of the tokens
   return( tokens );
}

////////////////////////////////////////////////////////////////////////////////
void
TokProcessCstr( const char *cstr, std::ostringstream &outSs )
{
   unsigned int             idx;
   int                      consumed;
   std::vector<std::string> tokens;

   // Parse the C string into a vector of token strings
   tokens = TokParseCstr( cstr );

   // for( idx=0; idx<tokens.size(); idx++){
   //    std::cout << "  token[" << idx << "] {" << tokens[idx] << "}\n";
   // }
    
   // If no tokens could be parsed clear the output string
   if( tokens.size() <= 0 ){
      outSs << "";
      return;
   }

   // Loop over all registered callbacks until one of them takes the line
   idx      = 0;
   consumed = 0;
   while( !consumed && (idx < gvCallbacks.size()) ){
      consumed = gvCallbacks[idx]->TokParse( tokens, outSs );
      idx++;
   }

   // If no callbacks recognized the command provide output with this feedback
   if( !consumed ){
     outSs << "No such command found\n";
   }
   
   return;
}

////////////////////////////////////////////////////////////////////////////////
/// Examples/Tests /////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#ifdef _UNIT_TEST_
class ExampleParserA : public TokCallback {

   // no need for any internal variable

   int TokParse( 
     std::vector<std::string> & arInputTokens , 
     std::ostringstream       & arOutStrSt 
   );

};

////////////////////////////////////////////////////////////////////////////////
int
ExampleParserA::TokParse( 
  std::vector<std::string> & arInputTokens , 
  std::ostringstream       & arOutStrSt 
)
{
   int consumed = 1;

   if( 0==arInputTokens[0].compare("help") ){
       arOutStrSt << "  help cmd found - this would be help text\n";
   }
   else if( 0==arInputTokens[0].compare("mboard" ) ){
       arOutStrSt << "  select mboard\n";
   }
   else{
       consumed = 0;
   }

   return( consumed );
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[] )
{
   char                     cstr[4096];
   std::ostringstream       oss;
   ExampleParserA           objA;

   sprintf(cstr,"first test line of input");
 
   TokRegister( &objA );
   do{

      oss.str(""); // Clear any residual output
      TokProcessCstr(cstr,oss);
      std::cout << oss.str();
      gets(cstr);

   } while(1);
}
#endif
