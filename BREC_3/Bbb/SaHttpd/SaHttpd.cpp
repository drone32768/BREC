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
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <microhttpd.h>
#include "InstModel.h"

////////////////////////////////////////////////////////////////////////////////
/// HTTP Interface /////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define         LOG_MASK_ACCESS  0x10000000
#define         LOG_MASK_FILE    0x08000000
unsigned int    gLogMask = 0;
InstModel      *gInstModel;

static char gErrPage[]=
"<html>"
"<head><title>File not found</title></head>"
"<body>File not found</body>"
"</html>";

static ssize_t
file_reader (void *cls, uint64_t pos, char *buf, size_t max)
{
  FILE *file = (FILE*)cls;

  if( gLogMask&LOG_MASK_FILE ){
      printf("file_reader:\n");
  }

  (void)  fseek (file, pos, SEEK_SET);
  return fread (buf, 1, max, file);
}

static void
free_callback (void *cls)
{
  if( gLogMask&LOG_MASK_FILE ){
      printf("file_callback:\n");
  }

  FILE *file = (FILE*)cls;
  fclose (file);
}

static int
ahc_access_handler (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data,
	  size_t *upload_data_size, void **ptr)
{
   struct MHD_Response *response;
   int                  ret;
   const char          *fname;
   FILE                *file;
   struct stat          buf;
#  define MAX_RESP_BYTES (2*65536)
   char                 rstr[MAX_RESP_BYTES];
 
   if( gLogMask&LOG_MASK_ACCESS ){
      printf("ahc_access_handler:%s <%s>\n", method, &url[1]);
   }

   if (0 != strcmp (method, MHD_HTTP_METHOD_GET)){
     return MHD_NO;             
   }

   /* reset when done */
   *ptr = NULL;                  


   //////////////////////////////////////// 
   if( 0==strcmp("setstate", &url[1]) ){
      char       *inStr;
      char       *name,*value;

      const char *vstr;
      char       *end;

      vstr = MHD_lookup_connection_value( 
                        connection, 
                        MHD_GET_ARGUMENT_KIND,
                        "v"
                      );
      // printf("ahc_access_handler:setstate v=\'%s\'\n",vstr);

      inStr = strdup( vstr );
      
      // NOTE: this doesn't parse a true JSON object
      // URL is expected to be setstate?v=name:value
      name  = strtok_r( inStr, ":", &end );
      value = strtok_r( NULL, "",   &end );

      // printf("ahc_access_handler: name=\'%s\',value=\'%s\'\n",name,value);

      gInstModel->SetState( name, value ); 

      sprintf(rstr,"ok");
      response = MHD_create_response_from_buffer (strlen (rstr),
 						  (void *) rstr,
 						  MHD_RESPMEM_MUST_COPY);
      ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
      MHD_destroy_response (response);

      free(inStr);
      return ret;
   }

   //////////////////////////////////////// 
   else if( 0==strcmp("getstate", &url[1]) ){

      gInstModel->GetState(rstr, sizeof(rstr) - 1 );
      response = MHD_create_response_from_buffer (strlen (rstr),
 						  (void *) rstr,
 						  MHD_RESPMEM_MUST_COPY);
      ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
      MHD_destroy_response (response);
      return ret;
   }

   //////////////////////////////////////// 
   else if( 0==strcmp("getdata", &url[1]) ){

      gInstModel->GetData(rstr, sizeof(rstr) - 1 );
      response = MHD_create_response_from_buffer (strlen (rstr),
 						  (void *) rstr,
 						  MHD_RESPMEM_MUST_COPY);
      ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
      MHD_destroy_response (response);
      return ret;
   }

   //////////////////////////////////////// 
   else if( 0==strcmp("cli", &url[1]) ){
      const char          *vstr;

      vstr = MHD_lookup_connection_value( 
                        connection, 
                        MHD_GET_ARGUMENT_KIND,
                        "v"
                      );

      printf("ahc_access_handler:cli v=\'%s\'\n",vstr);

      gInstModel->SndEvent(vstr);

      sprintf(rstr,"ok");
      response = MHD_create_response_from_buffer (strlen (rstr),
 						  (void *) rstr,
 						  MHD_RESPMEM_MUST_COPY);
      ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
      MHD_destroy_response (response);
      return ret;
   }

   //////////////////////////////////////// 
   else if( 0==url[1] ){
      fname = "index.html";
   }

   //////////////////////////////////////// 
   else {
      // TODO: this should limit the directory extent of file name
      fname = &url[1];
   }

   if (0 == stat (fname, &buf)){
     printf("Sending file <%s>\n",fname);
     file = fopen (fname, "rb");
   }
   else {
     file = NULL;
   }

   if (file == NULL) {
       response = MHD_create_response_from_buffer (strlen (gErrPage),
 						  (void *) gErrPage,
 						  MHD_RESPMEM_PERSISTENT);
       ret = MHD_queue_response (connection, MHD_HTTP_NOT_FOUND, response);
       MHD_destroy_response (response);
   }
   else {
       response = MHD_create_response_from_callback (
                             buf.st_size, 32 * 1024,     /* 32k page size */
                             &file_reader,
                             file,
                             &free_callback);
       if (response == NULL) {
 	  fclose (file);
 	  return MHD_NO;
       }
       ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
       MHD_destroy_response (response);
   }

   return ret;
}

void usage( int exit_code )
{
   fprintf(stderr,
"This utility implement a simple http based adc utility\n"
"Point a web browser at the platform address and specified port\n"
"-port <port>     http port to use (8080 default)\n"
"-config <fname>  configurtion file name\n"
);
   exit(0);
}

int
main (int argc, char **argv)
{
   struct MHD_Daemon *d;
   int                idx;
   int                port;
   const char        *cfgFname;
 
   // Default arguments
   port     = 8080;
   cfgFname = "sa.cfg";

   // Parse arguments
   idx = 1;
   while( idx< argc ){
       if( 0==strcmp(argv[idx],"-h") ){ 
          usage(0); 
          idx++;
          continue;
       }
       else if( 0==strcmp(argv[idx],"-help") ){ 
          usage(0); 
          idx++;
          continue;
       }
       else if( 0==strcmp(argv[idx],"-port") ){ 
          if((idx+1)>=argc ) usage(-1);
          port = atoi(argv[idx+1]);
          idx+=2;
          continue;
       }
       else if( 0==strcmp(argv[idx],"-config") ){ 
          if((idx+1)>=argc ) usage(-1);
          cfgFname = argv[idx+1];
          idx+=2;
          continue;
       }
       else{
          fprintf(stderr,"unrecogized arg %s\n",argv[idx]);
          idx++;
       }
   }

   // Show arguments
   printf("port   = %d\n",port);
   printf("config = %s\n",cfgFname);

   // Create and start the hardware model
   gInstModel = new InstModel();
   gInstModel->SetCfg( cfgFname );
   gInstModel->Start();

   // Create and start the http server
   d = MHD_start_daemon (
                // MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG,
                MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG,
                port,
                NULL,      NULL, 
                &ahc_access_handler, 
                (void*)gErrPage, 
                MHD_OPTION_END
       );

   if (d == NULL){
      return(-1);
   }

   while(1){ sleep(60); }
   return(0);
}
