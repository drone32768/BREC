The source code is structured as follows:

jtag_bs_f.cpp 
This is the core jtag access primitives for the actual F board.  The
interface is defined in ../JtagTools.  

Fxvc.cpp
This is the outer wrapper for starting the Xilinx virtual cable application
It relies on the jtag .o in this directory and the general xvc libary 
in ../JtagTools

Fxsvf.cpp
This is the main loop to drive the Xilinx xsvf player and includes 
code to read the image from local file.  It too relies on the jtag .o in 
this directory and the general xsvf library in ../JtagTools

Fboard.o
This is the primary interface library used to access and control the fpga
It exports a SPI interface.

pru00.p, pruinc.h pruconst.hp
These are the internal PRU support files for Fboard.o

Tool Use:

To connect with iMAPCT
  a) Start the virtual cable server
         arm/Fxvc
  b) In iMPACT connect cable, select plugin and enter
         xilinx_xvc host=192.168.0.170:2542 disableversioncheck=true

To load an .xsvf
  a) Use the xsvf player
         arm/Fxsvf Images/blinker.xsvf

To flash an image
  a) Load the host to flash spi image
         arm/Fxsvf Images/hspi.xsvf
  b) Verify spi access
         arm/Fflash -rdid
  c) Progam the image
         arm/Fflash -write Images/blinker.bin
         OR
         arm/Fflash -write Images/blinker.bit
  d) Read the current flash image
         arm/Fflash -read image.bin

Misc fpga/driver operations
  a) Pull program low 
         arm/Fctl -reset
  b) Show state of fpga and software
         arm/Fctl -show

