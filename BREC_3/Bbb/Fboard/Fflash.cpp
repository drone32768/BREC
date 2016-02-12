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
#include <arpa/inet.h> // for ntohs

#include "Util/mcf.h"
#include "Util/gpioutil.h"
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

   fbrd.SpiXferStream8( gSpiBf, 5 );

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

   fbrd.SpiXferStream8( gSpiBf, 2 );

   bytes[0]  = gSpiBf[1];

   return( 0 );
}

int
flash_wren()
{
   gSpiBf[0] = 0x06;

   fbrd.SpiXferStream8( gSpiBf, 1 );

   return(0);
}

int
flash_be()
{
   gSpiBf[0] = 0xC7;

   fbrd.SpiXferStream8( gSpiBf, 1 );

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

   fbrd.SpiXferStream8( gSpiBf, cnt );

   return(0);
}

int flash_read_page( unsigned int offset, unsigned char *bytes )
{
   int idx,cnt;

   cnt = 0;
   gSpiBf[cnt++] = 0x03;
   gSpiBf[cnt++] = (unsigned char)(offset>>16)&0xff ;
   gSpiBf[cnt++] = (unsigned char)(offset>>8 )&0xff ;
   gSpiBf[cnt++] = (unsigned char)(offset    )&0xff ;
   for(idx=0;idx<256;idx++){
      gSpiBf[cnt++] = 0x0;
   }

   fbrd.SpiXferStream8( gSpiBf, cnt );

   for(idx=0;idx<256;idx++){
      bytes[idx] = gSpiBf[ idx + 4 ];
   }

   return(0);
}

////////////////////////////////////////////////////////////////////////////////
/// Intermediate flash operations //////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int flash_establish_params()
{
   unsigned char bf[512];
   const char   *str;
   int           err;

   rdid(bf);

   err = 0;

   printf("Flash Parameters:\n");
   switch( bf[0] ){
       case 0x20:{ err|=0; str = "Micron/Numonyx"; break; }
       default:  { err|=1; str = "Unknown"; break; }
   }
   printf("    Manufacturer %s\n",str);

   switch( bf[1] ){
       case 0x20:{ err|=0; str = "M25"; break; }
       default:  { err|=2; str = "Unknown"; break; }
   }
   printf("    Memory Type  %s\n",str);

   switch( bf[2] ){
       case 0x13:{ err|=0; str = "P40/512kBytes"; break; }
       default:  { err|=4; str = "Unknown"; break; }
   }
   printf("    Capacity     %s\n",str);

   return( err );
}

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
flash_write( 
    unsigned char *pbytes, 
    int            pbyte_count, 
    int            verify,
    int            verbose 
)
{
   unsigned int  offset;
   unsigned char page_bytes[256];
   int           cnt,idx,err;

   if(verbose){ printf("Programming bytes %d \n",pbyte_count); }
   if(verbose){ printf("Verify is        %d \n",verify); }

   err = flash_establish_params();
   if( err ) return( err );

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

int
flash_read( 
    unsigned char *pbytes, 
    int            pbyte_count,
    int            verbose
)
{
   unsigned int  offset;
   unsigned char page_bytes[256];
   int           cnt,idx,err;

   if(verbose){ printf("Reading bytes %d \n",pbyte_count); }

   err = flash_establish_params();
   if( err ) return( err );

   offset = 0;
   while( offset < pbyte_count ){
       if(verbose){ printf("   rpage @ %d    \n",offset); }
       flash_read_page( offset, &(pbytes[offset]) );
       offset += 256;
   }
   if(verbose){ printf("End read.\n"); }

   return(0);
}

////////////////////////////////////////////////////////////////////////////////
/// File operations  ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// the exact specification of the headers is a bit sketchy, this is 
// based on the commonly published description
// NOTE: working buffer must be able to support at least 64k bytes
int
ReadBitHeaders( FILE *fh, unsigned char *bf, int verbose )
{
    unsigned short len;
    unsigned int   bytes;

    fread(bf,1,13,fh);

    if(verbose) printf("Bitstream Headers:\n");
    while( 1 ){
        bytes = fread(bf,1,1,fh);
        if( 1!=bytes ) return( bytes );
        if(verbose) printf("  Header 0x%02x (%c) ",*bf,*bf);

        if( 0x65 == *bf ){
           bytes = fread(bf,1,4,fh);
           if( 4!=bytes ) return( bytes );
           bytes = ntohl( *( unsigned short *)bf );
           if(verbose) printf("image bytes = %d\n",bytes);
           return( bytes );
        }
        else{
           bytes = fread(bf,1,2,fh);
           if( 2!=bytes ) return( bytes );
           len = ntohs( *( unsigned short *)bf );
           if(verbose) printf("[0x%04x] ",len,len);
        }
    
        bytes = fread(bf,1,len,fh);
        if( len!=bytes ) return( bytes );
        if(verbose) printf("\"%s\"\n",bf);
    }

}

const char* ReadFile( 
    const char    *fname, 
    unsigned char *image, 
    int            maxBytes,
    int           *rBytes,
    int            verbose
   )
{
   FILE          *fh;
   const char*    errStr = 0;
   int            idx;

   *rBytes = 0;

   // Parse the file based on suffix alone
   idx = strlen(fname);
   if( idx<5 ){
        return("No file suffix");
   }

   // This is a .bin file, just load it
   if( 
       'n'==fname[idx-1] &&
       'i'==fname[idx-2] &&
       'b'==fname[idx-3] &&
       '.'==fname[idx-4] 
     ){
        if(verbose) printf("Processing .bin file...\n");
        fh = fopen(fname,"rb");
        if(!fh) return("cannot open .bin file");
        *rBytes = fread(image,1,maxBytes,fh);
        fclose(fh);
   }

   // This is an .bit file, skip the headers and load it
   else if( 
       't'==fname[idx-1] &&
       'i'==fname[idx-2] &&
       'b'==fname[idx-3] &&
       '.'==fname[idx-4] 
     ){
        int ib;

        if(verbose) printf("Processing .bit file...\n");
        fh = fopen(fname,"rb");
        if(!fh) return("cannot open .bit file");
        ib = ReadBitHeaders(fh,image,1);
        if( ib<0 ){
           return("error in .bit header parsing");
        }
        *rBytes = fread(image,1,maxBytes,fh);
        fclose(fh);
   }

   // This is an unrecognized file
   else{
        return("Unrecognized file suffix");
   }

   return(errStr);
}

////////////////////////////////////////////////////////////////////////////////
/// Main application ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void usage( int exit_code )
{
    printf("-write  <fname>  write the file to spi flash\n");
    printf("-read   <fname>  read flash contents into file\n");
    printf("\n");
    printf("Other debug operations are:\n");
    printf("-usleep <M>  sleeps script execution for <M> micro seconds\n");
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

        else if( 0==strcmp(argv[idx], "-id") ){
            flash_establish_params();
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
            const char    *errStr    = "";
            
            image = (unsigned char*)malloc( max_bytes );
            errStr = ReadFile( argv[idx+1], image, max_bytes, &rbytes, 1 );
            if( rbytes>0 ){
                err=flash_write( image, 
                                   rbytes, 
                                   1 /*verify*/, 
                                   1 /*verbose*/ );
            }else{
                err = -100;
                printf("read %d bytes %s\n",rbytes,errStr);
            }
            printf("programmed %s with err = %d\n",argv[idx+1],err); 

            free(image);
            idx++;
        }

        else if( 0==strcmp(argv[idx], "-read") ){
            if( (idx+1) >= argc ){ usage(-1); }
            printf("reading flash to file %s\n",argv[idx+1]);

            unsigned char *image;
            FILE          *fh;
            int            rbytes;
            int            max_bytes = 1024*1024;
            int            err;

            rbytes = 512*1024;
            // rbytes = 1024; // for quick testing

            image = (unsigned char*)malloc( max_bytes );
            err = flash_read( image, rbytes, 1 /* verbose */ );

            if( err ){
                rbytes = 0;
                printf("failed to read flash\n");
            }

            fh = fopen(argv[idx+1],"wb+");
            if( fh!=NULL ){
                fwrite(image,1,rbytes,fh);
            }else{
                printf("failed to open output file %s\n",argv[idx+1]);
            }

            free(image);
            idx++;
        }
        idx++;
    }

    printf("Flash: Enter Exit\n");
}

