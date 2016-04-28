1.0 Disclaimer
NOTE: This is experimentation software - use at your own risk.

NOTE: Do not use this software on an open network. It is designed for 
flexibility in a lab and experimentation environment.  Numerous strings
are passed around as events and controll which are not fully verified or 
secure.  Multiple debug/service/diagnostic ports are open.

NOTE: In many instances, not all methods are fully implemented.  Only
enough to leverage the functionality required (e.g. ASCP interface).

This application is designed to be a linux radio server.  It is 
controlled by the open source ascp.  The following files are present

2.0 Summary
Interfaces and Application:
SdrSvr    - top level server application 
CliCtlIf  - command line control interface
AscpCtlIf - ascp control interface
AscpDisIf - ascp discovery interface
AscpDatIf - ascp data interface

Device and Signal Processing:
Device    - specific device hw control and signal processing

Utilities:
cli       - command line interface parser
mcf       - minimal control framework
net       - basic socket manipulation 

The following summarizes the organization of libraries and object.  To 
understand the core operation begin with Device - it is the central point
of control and organization between the higher level application and interfaces
and the lower level board specific devices.

       ------------------------------------------------------------------
      |                          SdrSvr                                  |
       ------------------------------------------------------------------
      |             |              |                   |                 |
      |  CliCtlIf   |   AscpDisIf  |   AscpDatIf       |   AscpCtlIf     |
      |             |              |                   |                 |
       ------------------------------------------------------------------
      |                         Device                                   |
       ------------------------------------------------------------------
      |            Various device software components                    |
       ------------------------------------------------------------------
      |                        Linux/GPIO/PRUSS                          |
       ------------------------------------------------------------------
      |                        Hardware                                  |
       ------------------------------------------------------------------

3.0 Minimal Control Framework
The minimal control framework allows an object to be instantiated which
has a dedicated pthread and an ascii command line text event method. Most
of the application is based around this allowing autonomous processing
as well as simple communication between threads.  This provides a good 
deal of flexiblity to rapidly re-assemble objects and processing for 
prototypes, demonstrations, and investigations.

The following lists the current events within the application:

ascp.dat.set-port        AscpDatIf       Set the port for data to be sent
ascp.dat.set-host        AscpDatIf       Set the host for data to be sent
ascp.dat.run             AscpDatIf       Begin sending data
ascp.dat.halt            AscpDatIf       Stop sending data

ascp.dis.set-port        AscpDisIf       Set UDP broadcast discovery port
ascp.dis.radio-name      AscpDisIf       Set radio name to respond with

device.tune-hz           Device          Set tuning frequency
device.sample-rate       Device          Set complex samples per second

