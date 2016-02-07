The source ocd is structured as follows:

jtag_bs.h
This is the definition of a minimum jtag interface.  Each board that
uses these libraries must provide this interface.

jtag_bs_sim.cpp
This is a simulated jtag interface.  It can be used with xvc and identifies
as a Spartan LX9

xsvfLib.*
This is the support library to play an xsv file through a jtag interface.
See the header file for unique circumstances

xvcLib.*
This is the Xilinx virtual cable daemon library.  It requires a board
specific jtag interface to be linked in to produce the final application

Sxvc.cpp
This is a sample of how to use the xvcLib and uses the simualted jtag
interface in this directory.

