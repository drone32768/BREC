
The tuner board (T board) is based on two parts with an I2C interface.
This interface is implemented with a 10 pin header following the 
same layout of the other boards previously used.  There are two main
control boards which support this: the I board and the F board (with
appropriate vhdl).  Each interface board requires slightly different
software to support the user space i2c library.

The layout of the software and hardware for the two primary cases is 
shown below:

    +---------------+
    |   Tutil       |
    +--------------------------------+
    |          Tboard                |
    +--------------------------------+
    |    mcp7725    |    max2112     |
    +--------------------------------+
    |              ui2c              |
    +--------------------------------+
    |    <GpioPin SW Interface>      |
    +--------------------------------+
    +--------------+  +--------------+
    |     Iboard   |  |     Bdc      |
    +--------------+  +--------------+
                      |    Fboard    | 
                      +--------------+
    **********************************
    **********************************
    +--------------+  +--------------+
    |    I HW      |  |   F HW       |
    +--------------+  +--------------+
                      |  Bdc07 Vhdl  |
                      +--------------+
    **********************************
    *  <I2C SDA/SCL HW Interface >   *
    **********************************
    +--------------------------------+
    |          T hardware            |
    +--------------------------------+

See the schematic for addressing and dac interface with tuner chip.
                      
