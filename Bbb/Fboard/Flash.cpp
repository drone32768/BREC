/*
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Util/mcf.h"
#include "../Util/gpioutil.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int      fspiDbg = 0;

GpioUtil hss;
GpioUtil hsclk;
GpioUtil hmosi[2];
GpioUtil hmiso[2];

int
fspi_init( )
{
    hss.Define(      117 );
    hsclk.Define(    110 );
    hmosi[0].Define( 113 );
    hmosi[1].Define( 115 );
    hmiso[0].Define( 111 );
    hmiso[1].Define( 112 );

    hss.Export();
    hsclk.Export();
    hmosi[0].Export();
    hmosi[1].Export();
    hmiso[0].Export();
    hmiso[1].Export();

    hss.SetDirInput(      0 );
    hsclk.SetDirInput(    0 );
    hmosi[0].SetDirInput( 0 );
    hmosi[1].SetDirInput( 0 );
    hmiso[0].SetDirInput( 1 );
    hmiso[1].SetDirInput( 1 );

    hss.Open();
    hsclk.Open();
    hmosi[0].Open();
    hmosi[1].Open();
    hmiso[0].Open();
    hmiso[1].Open();

    hss.Set(1);
    hsclk.Set(0);

    return(0);
}

int 
fspi_select()
{
    hss.Set( 0 );
}

int 
fspi_deselect()
{
    hss.Set( 1 );
}

int 
fspi_write_byte( int wval )
{
    int rval;
    int obit,ibit,idx;

    if( fspiDbg ){
       printf("fspi_write: wval = 0x%04x\n",wval);
    }

    rval    = 0;

    // expecting: sclk=0, ss=0

    for( idx=7; idx>=0; idx-- ){
        if( fspiDbg ){
            printf("idx[%d]\n",idx);
        }

        if( wval&0x80 ) obit = 1;
        else            obit = 0;

        if( fspiDbg ){
            printf("   obit = %d\n",obit);
        }
        hmosi[0].Set( obit );
        hsclk.Set( 1 );
        ibit = hmiso[0].Get( );
        if( fspiDbg ){
            printf("   ibit = %d\n",ibit);
        }

        hsclk.Set( 0 );

        rval = (rval<<1) | ibit;
        wval = (wval<<1);
    }

    // expecting: sclk=0, ss=1

    return(rval);
}

int 
fspi_read_byte( int wval )
{
    int rval;
    int obit,ibit,idx;


    rval    = 0;

    // expecting: sclk=0, ss=0

    for( idx=7; idx>=0; idx-- ){
        if( fspiDbg ){
            printf("idx[%d]\n",idx);
        }

        if( wval&0x80 ) obit = 1;
        else            obit = 0;

        if( fspiDbg ){
            printf("   obit = %d\n",obit);
        }
        hmosi[0].Set( obit );
        hsclk.Set( 1 );
        ibit = hmiso[0].Get( );
        if( fspiDbg ){
            printf("   ibit = %d\n",ibit);
        }

        hsclk.Set( 0 );

        rval = (rval<<1) | ibit;
        wval = (wval<<1);
    }

    // expecting: sclk=0, ss=1

    if( fspiDbg ){
       printf("fspi_read: rval = 0x%04x\n",rval);
    }

    return(rval);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int
rdid( unsigned char *bytes )
{
   unsigned short rb;

   fspi_select();
   fspi_write_byte( 0x9F );
   bytes[0] = fspi_read_byte( 0 );
   bytes[1] = fspi_read_byte( 0 );
   bytes[2] = fspi_read_byte( 0 );
   bytes[3] = fspi_read_byte( 0 );
   fspi_deselect();

   return(0);
}

int
flash_rdsr( unsigned char *bytes )
{
   unsigned short rb;

   fspi_select();
   fspi_write_byte( 0x05 );
   bytes[0] = fspi_read_byte( 0 );
   fspi_deselect();

   return( rb );
}

int
flash_wren()
{
   fspi_select();
   fspi_write_byte( 0x06 );
   fspi_deselect();

   return(0);
}

int
flash_be()
{
   fspi_select();
   fspi_write_byte( 0xC7 );
   fspi_deselect();

   return(0);
}

int flash_write_page( unsigned int offset, unsigned char *bytes )
{
   int idx;

   fspi_select();
   fspi_write_byte( 0x02 );
   fspi_write_byte( (offset>>16)&0xff  );
   fspi_write_byte( (offset>>8 )&0xff  );
   fspi_write_byte( (offset    )&0xff  );
   for(idx=0;idx<256;idx++){
       fspi_write_byte(bytes[idx]);
   }
   fspi_deselect();

   return(0);
}

int flash_read_page( unsigned int offset, unsigned char *bytes )
{
   int idx;
   fspi_select();
   fspi_write_byte( 0x03 );
   fspi_write_byte( (offset>>16)&0xff  );
   fspi_write_byte( (offset>>8 )&0xff  );
   fspi_write_byte( (offset    )&0xff  );
   for(idx=0;idx<256;idx++){
       bytes[idx] = fspi_read_byte(0);
   }
   fspi_deselect();
   return(0);
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
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void usage( int exit_code )
{
    printf("-usleep <M>  sleeps script execution for <M> micro seconds\n");
    printf("-write  <N>  write the 16 bit word N\n");
    printf("-echo   <str> echo string <str>\n");
    printf("-rdid        read and display device id\n");
    printf("-rdsr        read and display device status register\n");
    printf("-wen         set write enable\n");
    printf("-be          start bulk erase\n");
    printf("\n");
    printf("e.g. -write 0x00001\n");
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

    fspi_init( );
 
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

