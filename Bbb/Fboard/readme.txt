The source code is structured as follows:

jtag_bs_f.cpp 
This is the core jtag access primitives for the actual F board.  The
interface is defined in ../JtagTools

Fxvc.cpp
This is the outer wrapper for starting the Xilinx virtual cable application
It relies on the jtag .o in this directory and the general xvc libary 
in ../JtagTools

Fxsvf.cpp
This is the main loop to drive the Xilinx xsvf player and includes 
code to read the image from local file.  It too relies on the jtag .o in 
this directory and the general xsvf library in ../JtagTools
