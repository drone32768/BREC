#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "Devs.h"

//------------------------------------------------------------------------------
static Devs *gpDevs = NULL;

//------------------------------------------------------------------------------
Devs *Dp()
{
    if( !gpDevs ){
       gpDevs = new Devs();
    }
    return( gpDevs );
}

//------------------------------------------------------------------------------
Devs::Devs()
{
}

//------------------------------------------------------------------------------
int Devs::OpenI()
{
    Mboard        *mbrd;
    Gpio6PinGroup *g6pg;

    printf("*********** Devs::Open(I) Begin ***************************\n");
    mIbrd = new Iboard();
    mIbrd->Open();

    printf("*********** Devs::Open Starting M board @ 0 ************\n");
    g6pg = mIbrd->AllocPort( 0 );
    mIbrd->EnablePort( 0, 1 ); 

    mbrd = new Mboard();
    mbrd->Open( g6pg );
    mLo[0] = mbrd->GetAdf4351( 0 );

    printf("*********** Devs::Open End *****************************\n");
    return( 0 );
}

//------------------------------------------------------------------------------
int Devs::Open()
{
    Bboard *bcBoard;

// TODO

    if( FindCapeByName("brecA")>0  ){

        printf("*********** Devs::Open(A) Begin ***************************\n");
        bcBoard = new Bboard();
        bcBoard->Open();

        printf("*********** Devs::Open Starting B board ****************\n");
        mLo[0] = bcBoard->GetAdf4351( 0 );
        mLo[0]->SetFrequency(  1575000000 );
        mLo[0]->SetLog(0);

        printf("*********** Devs::Open Starting C board ****************\n");
        mLo[1] = bcBoard->GetAdf4351( 1 );
        mLo[1]->SetFrequency(  1585750000 );
        mLo[1]->SetLog(0);

        printf("*********** Devs::Open Starting A board ****************\n");
        mAdc   = new Aboard();
        mAdc->Open();
        mAdc->StartPrus();
        mAdc->SetComplexSampleRate( 5000000 );
    
        printf("*********** Devs::Open Starting R board ****************\n");
        mRbrd = new Rboard();
        mRbrd->SetDev(0);
        mRbrd->Open();
        mRbrd->SetChannel( 0 );

    }

    if( FindCapeByName("brecI")>0  ){
        printf("*********** Devs::Open Starting I board ****************\n");
    }

    if( FindCapeByName("brecH")>0  ){
        printf("*********** Devs::Open Starting H board ****************\n");
        mAdc   = new Hboard();
        mAdc->Open();
        mAdc->StartPrus();
        mAdc->SetComplexSampleRate( 5000000 );
    }

    if( FindCapeByName("brecX")>0  ){
        printf("*********** Devs::Open Starting X board ****************\n");
        mAdc = new Xboard();
        mAdc->Open();
        mAdc->StartPrus();
        mAdc->SetComplexSampleRate( 5000000 );
        ((Xboard*)( mAdc ))->SetSource( 0 );
        ((Xboard*)( mAdc ))->SetFrequency( 10640000 );
    }

    // x86 simulation
#   ifdef TGT_X86
    {
        printf("*********** Devs::Open Starting X board ****************\n");
        mAdc = new Xboard();
        mAdc->Open();
        mAdc->StartPrus();
        mAdc->SetComplexSampleRate( 5000000 );
        // ((Xboard*)( mAdc ))->SetSource( 5 );
        ((Xboard*)( mAdc ))->SetSource( 0 );
        ((Xboard*)( mAdc ))->SetFrequency( 10640000 );
    }
#   endif

    printf("*********** Devs::Open End *****************************\n");

    return(0);
}

//------------------------------------------------------------------------------

