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

#include "SimpleTable.h"

////////////////////////////////////////////////////////////////////////////////
SimpleTable::SimpleTable()
{
    mSimpleTableNameStr = NULL;
    mSize       = 32;
    mEntries    = 0;
    mXvals      = (double*)malloc( sizeof(double) * mSize );
    mYvals      = (double*)malloc( sizeof(double) * mSize );
}

////////////////////////////////////////////////////////////////////////////////
void
SimpleTable::SetName( const char *name )
{
    mSimpleTableNameStr = strdup(name);
}

////////////////////////////////////////////////////////////////////////////////
void
SimpleTable::Print()
{
    int idx;
    printf("x,y,name=%s,count=%d,size=%d\n",mSimpleTableNameStr,mEntries,mSize);
    for(idx=0;idx<mEntries;idx++){
        printf("%f , %f\n",mXvals[idx],mYvals[idx]);
    }
}

////////////////////////////////////////////////////////////////////////////////
int 
SimpleTable::AddEntry( double x, double y )
{
    int idx;

    if( (mEntries+1) >= mSize ){
        printf("Resizing table %d\n",mEntries);
        mSize = mSize * 2;
        mXvals = (double*)realloc( mXvals, sizeof( double )* mSize );
        mYvals = (double*)realloc( mXvals, sizeof( double )* mSize );
    }

    mXvals[ mEntries ] = x;
    mYvals[ mEntries ] = y;         
    idx = mEntries;
    mEntries++;

    return( idx );
}

////////////////////////////////////////////////////////////////////////////////
double 
SimpleTable::Interp( double x )
{
    int    idx;
    double Dx,Dy,dx,dy,y;

    idx = 0;
    while( (mXvals[idx] < x) && (idx<mEntries) ){
        idx++;
    }

    if( 0==idx ){
        return( mYvals[0] );
    }

    if( mEntries==idx ){
        return( mYvals[mEntries-1] );
    }

    Dx = mXvals[idx] - mXvals[idx-1];
    Dy = mYvals[idx] - mYvals[idx-1];

    dx = x - mXvals[ idx-1 ];
    dy = Dy*(dx / Dx);

    y  = mYvals[ idx-1 ] + dy;
    
    return(y);
}

////////////////////////////////////////////////////////////////////////////////
#ifdef UNIT_TEST 
int test00()
{
    double      x;
    SimpleTable tbl;

    tbl.SetName("test00");
    tbl.AddEntry( 10, 50 );
    tbl.AddEntry( 20, 60 );
    tbl.AddEntry( 30, 70 );
    tbl.AddEntry( 40, 80 );
    tbl.Print();

    x = 9.0;
    printf("x=%f,y=%f\n",x, tbl.Interp( x ) );

    x = 41.0;
    printf("x=%f,y=%f\n",x, tbl.Interp( x ) );

    x = 20.1;
    printf("x=%f,y=%f\n",x, tbl.Interp( x ) );

    x = 20.5;
    printf("x=%f,y=%f\n",x, tbl.Interp( x ) );

    x = 29.9;
    printf("x=%f,y=%f\n",x, tbl.Interp( x ) );

    return(0);
}

int
main( int argc, char *argv[] )
{
    test00();
}
#endif
