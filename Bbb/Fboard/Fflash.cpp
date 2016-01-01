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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"
#include "Fboard.h"

////////////////////////////////////////////////////////////////////////////////
/// Basic flash operations /////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
Fboard        fbrd;
unsigned char gSpiBf[512];

int
rdid( unsigned char *bytes )
{
   gSpiBf[0] = 0x9F;
   gSpiBf[1] = 0x00;
   gSpiBf[2] = 0x00;
   gSpiBf[3] = 0x00;
   gSpiBf[4] = 0x00;

   fbrd.SpiXfer( gSpiBf, 5 );

   bytes[0]  = gSpiBf[1];
   bytes[1]  = gSpiBf[2];
   bytes[2]  = gSpiBf[3];
   bytes[3]  = gSpiBf[4];

   return(0);
}

int
flash_rdsr( unsigned char *bytes )
{
   gSpiBf[0] = 0x05;
   gSpiBf[1] = 0x00;

   fbrd.SpiXfer( gSpiBf, 2 );

   bytes[0]  = gSpiBf[1];

   return( 0 );
}

int
flash_wren()
{
   gSpiBf[0] = 0x06;

   fbrd.SpiXfer( gSpiBf, 1 );

   return(0);
}

int
flash_be()
{
   gSpiBf[0] = 0xC7;

   fbrd.SpiXfer( gSpiBf, 1 );

   return(0);
}

int flash_write_page( unsigned int offset, unsigned char *bytes )
{
   int idx,cnt;

   cnt = 0;
   gSpiBf[cnt++] = 0x02;
   gSpiBf[cnt++] = (unsigned char)(offset>>16)&0xff ;
   gSpiBf[cnt++] = (unsigned char)(offset>>8 )&0xff ;
   gSpiBf[cnt++] = (unsigned char)(offset    )&0xff ;
   for(idx=0;idx<256;idx++){
      gSpiBf[cnt++] = bytes[idx];
   }

   fbrd.SpiXfer( gSpiBf, cnt );

   return(0);
}

int flash_read_page( unsigned int offset, unsigned char *bytes )
{
   int idx,cnt;

   cnt = 0;
   gSpiBf[cnt++] = 0x02;
   gSpiBf[cnt++] = (unsigned char)(offset>>16)&0xff ;
   gSpiBf[cnt++] = (unsigned char)(offset>>8 )&0xff ;
   gSpiBf[cnt++] = (unsigned char)(offset    )&0xff ;
   for(idx=0;idx<256;idx++){
      gSpiBf[cnt++] = 0x0;
   }

   fbrd.SpiXfer( gSpiBf, cnt );

   for(idx=0;idx<256;idx++){
      bytes[idx] = gSpiBf[ idx + 4 ];
   }

   return(0);
}

////////////////////////////////////////////////////////////////////////////////
/// Intermediate flash operations //////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int flash_wait_wip_clear()
{
   unsigned char b;
   int           cnt;

   cnt = 0;
   while( cnt<100 ){
       flash_rdsr(&b);
       if( b&0x01 ){
          us_sleep( 10 );
       }
       else{
          return(0);
       }
   }

   return(-1); 
}

int
flash_program( 
    unsigned char *pbytes, 
    int            pbyte_count, 
    int            verify,
    int            verbose 
)
{
   unsigned int  offset;
   unsigned char page_bytes[256];
   int           cnt,idx;

   if(verbose){ printf("Progamming bytes %d \n",pbyte_count); }
   if(verbose){ printf("Verify is        %d \n",verify); }

   rdid( page_bytes );
   printf("mfg  = 0x%02x\n",page_bytes[0]);
   printf("typ  = 0x%02x\n",page_bytes[1]);
   printf("cap  = 0x%02x\n",page_bytes[2]);
   printf("cfdl = 0x%02x\n",page_bytes[3]);

   if(verbose){ printf("Starting bulk erase\n"); }
   flash_wren();
   flash_be();
   cnt = 0;
   while( 1 ){
      flash_rdsr(page_bytes);
      if( page_bytes[0] & 0x01 ){
          if(verbose){ printf("   WIP set %d...\n",cnt); }
          us_sleep(1000000);
          cnt++;
      }
      else{
          break;
      }
      if( cnt > 20 ){
         if(verbose){ printf("Erase timeout - device not programmed\n"); }
         return( -1 );
      }
   }
   if(verbose){ printf("RDSR = 0x%02x\n",page_bytes[0]); }
   if(verbose){ printf("Bulk erase complete.\n"); }

   if(verbose){ printf("Begin programming...\n"); }
   flash_wren();
   flash_rdsr(page_bytes);
   if(verbose){ printf("RDSR = 0x%02x\n",page_bytes[0]); }
   offset = 0;
   while( offset < pbyte_count ){
       if(verbose){ printf("   ppage @ %d    \n",offset); }

       flash_wren();
       flash_write_page( offset, pbytes+offset );
       offset += 256;
       flash_wait_wip_clear();

   }
   if(verbose){ printf("End programming.\n"); }
   
   if(!verify){
      return(0);
   }

   if(verbose){ printf("Begin verify...\n"); }
   offset = 0;
   while( offset < pbyte_count ){
       if(verbose){ printf("   vpage @ %d    \n",offset); }
       flash_read_page( offset, page_bytes );
       if( memcmp(page_bytes,pbytes+offset,256) ){
          if(verbose){ printf("      mismatch in this page\n"); }
          printf("idx, actual,   expected\n");
          for(idx=0;idx<256;idx++){
              printf("[%03d], 0x%02x(%03d), 0x%02x(%0d)\n",
                     idx,
                     page_bytes[idx],
                     page_bytes[idx],
                     pbytes[offset+idx],
                     pbytes[offset+idx]
              );
          }
          return(-2);
       }
       offset += 256;
   }
   if(verbose){ printf("End verify.\n"); }

   return(0);
}

////////////////////////////////////////////////////////////////////////////////
/// Main application ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void usage( int exit_code )
{
    printf("-write  <fname>  write the file to spi flash\n");
    printf("\n");
    printf("Other debug operations are:\n");
    printf("-usleep <M>  sleeps script execution for <M> micro seconds\n");
    printf("-echo   <str> echo string <str>\n");
    printf("-rdid        read and display device id\n");
    printf("-rdsr        read and display device status register\n");
    printf("-wen         set write enable\n");
    printf("-be          start bulk erase\n");
    printf("\n");
    printf("e.g. -rdid -write blinker.bin\n");
    exit( exit_code );
}

int
main( int argc, char *argv[] )
{
    int            idx;
    int            val;
    char          *end;
    unsigned char  bytes[256];

    printf("Flash: Enter Main\n");

    fbrd.Open();
    fbrd.Show();
 
    idx = 1;
    while( idx < argc ){

        if( 0==strcmp(argv[idx], "-help") ){
            usage(0);
        }

        else if( 0==strcmp(argv[idx], "-h") ){
            usage(0);
        }

        else if( 0==strcmp(argv[idx], "-usleep") ){
            if( (idx+1) >= argc ){ usage(-1); }
            val = strtol(argv[idx+1],&end,0);
            us_sleep( val );
        } 

        else if( 0==strcmp(argv[idx], "-echo") ){
            if( (idx+1) >= argc ){ usage(-1); }
            printf("%s\n",argv[idx+1]);
        } 

        else if( 0==strcmp(argv[idx], "-rdid") ){
            rdid( bytes );
            printf("mfg  = 0x%02x\n",bytes[0]);
            printf("typ  = 0x%02x\n",bytes[1]);
            printf("cap  = 0x%02x\n",bytes[2]);
            printf("cfdl = 0x%02x\n",bytes[3]);
        }

        else if( 0==strcmp(argv[idx], "-rdsr") ){
            flash_rdsr( bytes );
            printf("rdsr = 0x%02x\n",bytes[0]);
        }

        else if( 0==strcmp(argv[idx], "-wren") ){
            flash_wren();
            printf("set write enable\n");
        }

        else if( 0==strcmp(argv[idx], "-be") ){
            flash_be();
            printf("initiated block erase, check with -rdsr\n");
        }

        else if( 0==strcmp(argv[idx], "-write") ){
            if( (idx+1) >= argc ){ usage(-1); }
            printf("writing file %s\n",argv[idx+1]);

            unsigned char *image;
            FILE          *fh;
            int            rbytes;
            int            max_bytes = 1024*1024;
            int            err;

            image = (unsigned char*)malloc( max_bytes );
            fh = fopen(argv[idx+1],"r");
            if( fh!=NULL ){
                rbytes = fread(image,1,max_bytes,fh);
            }else{
                printf("failed to open input file %s\n",argv[idx+1]);
                rbytes = -1;
            }
            if( rbytes>0 ){
                err=flash_program( image, 
                                   rbytes, 
                                   1 /*verify*/, 
                                   1 /*verbose*/ );
            }else{
                err = -100;
                printf("read %d bytes\n",rbytes);
            }
            printf("programmed %s with err = %d\n",argv[idx+1],err); 

            free(image);
            idx++;
        }

        idx++;
    }

    printf("Flash: Enter Exit\n");
}

