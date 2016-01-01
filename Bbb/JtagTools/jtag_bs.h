/* 
 * These are the required jtag methods needed by general utilities which
 * each board must provide.
 */
extern void          jtag_bs_open();
extern void          jtag_bs_close();

extern void          jtag_bs_set_tms( int v );
extern void          jtag_bs_set_tdi( int v );
extern void          jtag_bs_set_tck( int v );
extern unsigned char jtag_bs_get_tdo(       );

