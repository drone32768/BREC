#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

int main( int argc, char *argv[] )
{
    FILE *inFp;
    int   wc,nb;
    char  ch;
    const char  *inputFname;
    const char  *varName;

    inputFname = "xyz.bin";
    varName    = "xyz00";

    wc=1;
    while(wc<argc){
        if( 0==strcmp("-help",argv[wc]) ){
           printf("-f <filename>     input file\n");
           printf("-n <varname>      variable name\n");
        }
        else if ( 0==strcmp("-f",argv[wc]) ){
           wc++;
           inputFname = argv[wc];
        }
        else if ( 0==strcmp("-n",argv[wc]) ){
           wc++;
           varName = argv[wc];
        }
        wc++;
    }

    inFp = fopen(inputFname,"rb");
    if( !inFp ){
        fprintf(stderr,"first argument is in put file\n");
        return(-1);   
    }

    printf("static unsigned char %s[] = {\n",varName);
    wc = 0;
    while( 1 ){
        nb = fread(&ch,1,1,inFp);
        if( 0 == nb ) break;
        printf("0x%02x, ",ch);
	wc++;
        if( 0==(wc%8) ) printf("\n");
    }
    printf("\n};\n");
}

