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
#ifndef __TOK_H__
#define __TOK_H__

#include <string>
#include <iostream>
#include <sstream>
#include <vector>

/**
This class provides a C++ Tokenizer centered around C string command line
inputs.  It is designed to parse a command line into tokens which are
made available to registerd callbacks.  If a callback recognizes the command
it acts on it and places the output of the command into an output string
stream.  After the first registered callback accepts the command callback
invocations are stopped.

An object must inherit from the TokCallback interface and provide a method
TokParse().  The object can register itself via TokRegister() and will
then be invoked on inputs.  The object's TokParse must return 0 if it did
not recognize the token vector, non 0 if it did and acted on the commands.

Processing is driven by invoking TokProcessCstr with the input C string and
a reference to the output string stream where the command results are to
be placed.

The following is a brief summary of intended use (See the .cpp for an example
and test).

On startup:
     // objA is an instance of an oject implementing TokCallback interface
     // with TokParse defined.
     TokRegister( &objA )

On C string input:
     TokProcessCstr(cstr,oss);
     std::cout << oss.str();

*/

/**
 * Classes which have a tokenizer callback should inherit from this
 * interface and define a callback method (which is registered)
 */
class TokCallback {

public:

   /**
    * This method is given a vector of strings (tokens).   The vector is 
    * guarenteed to contain at least one token.  This method is expected
    * to parse the token if it is valid and produce output for the tokens
    * in the provided output string stream.  If the tokens do not apply
    * the method should do nothing and return 0.  If the tokens applied and
    * were processed then a non zero value should be returned.
    */
   virtual int TokParse( 
      std::vector<std::string> & arInputTokens , 
      std::ostringstream       & arOutStrSt ) = 0;
};

/**
 * Register a callback.  Object must provide a TokCallback interface
 */
void TokRegister( TokCallback *tokCb );

/**
 * Process the input C string by parsing it into tokens, calling any
 * register callbacks and producing the specific output in the provided
 * output string stream.
 */
void TokProcessCstr( const char *cstr, std::ostringstream &outSs );

#endif
