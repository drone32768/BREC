/*
 *
 * This is somewhat convoluted but ... it works.
 * 
 * The goal is to leverage the Xilinx 058 application note code, however,
 * its licenses does not allow including a copy with the rest of this
 * source code.  
 *
 * To that end it is treated like any other large scale library where the
 * source is too large to push around.  A copy is installed on all of the
 * build machines and the libraries/headers are referenced from the install.
 *
 * Unfortunately, there are changes required to those files to get them
 * to work the way they are applied here.  The changes are minor (factoring
 * and strict syntax correcting ).  By maintaining a source code "install"
 * this library can be built one time and just linked against.
 *
 * Register and download the xapp058 source code and configure build process so
 * it has access to those files.
 *
 * Each of the .c files was change to .cpp and minor cleanup conducted
 * to get things to compile cleanly and without main()
 *
 */

// Client may access these
extern int xsvfExecute();
extern int xsvf_iDebugLevel;

// Client must supply this
extern unsigned char xsvf_next_byte();

