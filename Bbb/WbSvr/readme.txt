1.0 Disclaimer
NOTE: This is experimentation software - use at your own risk.

NOTE: Do not use this software on an open network. It is designed for 
flexibility in a lab and experimentation environment.  Numerous strings
are passed around as events and control which are not fully verified or 
secure.  Multiple debug/service/diagnostic ports are open.

2.0 Summary
The following diagram illustrates the sub-applications and interfaces within
the workbench application.  Each box is a mcf thread.  The device object
is a singleton instance.  No mutex is provided (yes you can get yourself 
in trouble).  This is by design and allows command line access via the
core Wb command line interface.

  <----------------------------------------------------- 
     UDP(samp)                                          | 
                -----------------                -----------------
               |                 |              |                 | 
  <----------->|   AdcCtlIf      |------------->|  AdcDatIf       |---- 
     TCP(cli)  |                 |    Evt       |                 |    |
                -----------------                -----------------     |
                                                                       |
                -----------------                                      |
               |                 |                                     |
  <----------->|   SiCtlIf       |-------------------------------------|
     TCP(cli)  |                 |                                     |
                -----------------                                      | 
                                                                       |
                                                                       |
                -----------------                                      |
               |                 |                                     |
  <----------->|   SgCtlIf       |-------------------------------------|
     TCP(cli)  |                 |                                     |
                -----------------                                      | 
                                                                       |
                                         ^ (injected evt)              |
                -----------------        |                             |
               |                 |-------                              |
  <----------->|   WbCtlIf       |-------------------------------------|
     TCP(cli)  |                 |                                     |
                -----------------                                      | 
                                                                       V
                                                                    +++++++++
                                                                   +  Devs   +
                                                                    +++++++++
AdcCtl/AdcDat
Provides UDP stream of ADC samples with limited control of gains.

SiCtl
Serves as spectral investigator. Can be used as a limited spectrum 
analyzer or spectrum analyzer with tracking generator.  See full documentation
this is not a real instrument, rather, an approximation using available
hardware. (its called an investigator and not an analyzer for a reason)

Sg
Serves as a simple signal generator.  Limited bandwidth and output level
control.

Wb  
Provides full control directly of the hardware.  Can be used stand alone
with scripts or while other applications are executing for experiments.
Includes limited built in tests and calibration routines.
