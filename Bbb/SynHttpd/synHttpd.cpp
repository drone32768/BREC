#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <microhttpd.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include "../Bboard/Bboard.h"
#include "../Adf4351/Adf4351.h"

Adf4351 *gSyn;

static void InitHw()
{
    Bboard *bBoard;

    bBoard = new Bboard();
    bBoard->Open();

    gSyn = bBoard->GetAdf4351( 0 );
    gSyn->SetFrequency( 100000000 );
    gSyn->SetLog( 0xffff );
    gSyn->SetMtld( 1 );
    gSyn->SetAuxPower( 0 );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int       gServerExit;
long long gFreqHz;

////////////////////////////////////////////////////////////////////////////////
#define PAGE "<html><head><title>File not found</title></head><body>File not found</body></html>"

////////////////////////////////////////////////////////////////////////////////
static ssize_t
file_reader (void *cls, uint64_t pos, char *buf, size_t max)
{
  FILE *file = (FILE*)cls;

  printf("file_reader:\n");

  (void)  fseek (file, pos, SEEK_SET);
  return fread (buf, 1, max, file);
}

////////////////////////////////////////////////////////////////////////////////
static void
free_callback (void *cls)
{
  FILE *file = (FILE*)cls;
  fclose (file);
}

////////////////////////////////////////////////////////////////////////////////
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
   char                 rstr[1024];
 
   printf("ahc_access_handler:%s <%s>\n", method, &url[1]);

   if (0 != strcmp (method, MHD_HTTP_METHOD_GET)){
     return MHD_NO;             
   }

/*
   static int           aptr;
   if (&aptr != *ptr) {
       *ptr = &aptr;
       return MHD_YES;
   }
*/
   *ptr = NULL;                  /* reset when done */

   //////////////////////////////////////// 
   if( 0==strcmp("swrestart",&url[1]) ){
       sprintf(rstr,"ok");
       response = MHD_create_response_from_buffer (strlen (rstr),
 						  (void *) rstr,
 						  MHD_RESPMEM_MUST_COPY);
       ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
       MHD_destroy_response (response);

      gServerExit = 1;
      return MHD_YES;
   }

   //////////////////////////////////////// 
   if( 0==strcmp("hwrestart",&url[1]) ){
      gServerExit = 1;
      return MHD_YES;
   }

   //////////////////////////////////////// 
   if( 0==strcmp("shutdown",&url[1]) ){
      gServerExit = 1;
      return MHD_YES;
   }

   //////////////////////////////////////// 
   if( 0==strcmp("savecfg",&url[1]) ){
       sprintf(rstr,"ok");
       response = MHD_create_response_from_buffer (strlen (rstr),
 						  (void *) rstr,
 						  MHD_RESPMEM_MUST_COPY);
       ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
       MHD_destroy_response (response);
       return ret;
   }

   //////////////////////////////////////// 
   else if( 0==strcmp("setfreq", &url[1]) ){
      const char *vstr;
      char       *end;

      vstr = MHD_lookup_connection_value( 
                        connection, 
                        MHD_GET_ARGUMENT_KIND,
                        "f"
                      );
      printf("f=%s\n",vstr);

      gFreqHz = strtoll( vstr, &end, 0 );
      sprintf(rstr,"%lld",gFreqHz);
      gSyn->SetFrequency( gFreqHz );

      response = MHD_create_response_from_buffer (strlen (rstr),
 						  (void *) rstr,
 						  MHD_RESPMEM_MUST_COPY);
      ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
      MHD_destroy_response (response);
      return ret;
   }

   //////////////////////////////////////// 
   else if( 0==strcmp("getstate", &url[1]) ){

       time_t now;
       char   timeStr[128];

       time( &now );
       ctime_r( &now, timeStr );
       timeStr[ strlen(timeStr) - 1 ] = 0; // remove new line

        
       sprintf(rstr,"{ "
                    "\"lock\" : %d, "
                    "\"freq\" : %llu,  "
                    "\"time\" : \"%s\"  "
                    " }",
                       gSyn->GetLock(),
                       gSyn->GetFrequency(),
                       timeStr
              );

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
      fname = &url[1];
   }

   if (0 == stat (fname, &buf)){
     file = fopen (fname, "rb");
   }
   else {
     file = NULL;
   }

   if (file == NULL) {
       response = MHD_create_response_from_buffer (strlen (PAGE),
 						  (void *) PAGE,
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

////////////////////////////////////////////////////////////////////////////////
/**
 *
 */
int
main (int argc, char **argv)
{
   struct MHD_Daemon *d;

   if (argc != 2){
      printf ("%s PORT\n", argv[0]);
      return 1;
   }

   InitHw();

   gServerExit = 0;
   d = MHD_start_daemon (
                // MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG,
                MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG,
                atoi (argv[1]),
                NULL,      NULL, 
                &ahc_access_handler, 
                (void*)PAGE, 
                MHD_OPTION_END
       );

   if (d == NULL){
      return 1;
   }

   while( !gServerExit ){
      sleep(1);
   }

   MHD_stop_daemon (d);
   return 0;
}
