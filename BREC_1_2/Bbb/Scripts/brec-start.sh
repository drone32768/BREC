#!/bin/bash

###############################################################################
#Start up sequence:
#The overall approach to startup sequence is to chain scripts to the
#linux init sequence. The following are notes on how to configure a script
#with systemd
#
#1.0 Create script /home/root/brec-start.sh
#    1.1 Use absolute paths unless working directory set in service file
#    1.2 The forking (below) allows this script to start applications in 
#        the background and then exit.
#2.0 Establish the service with systemd
#    2.1 Edit the file:
#        /lib/systemd/brec-start.service
#    2.2 Include the following lines:
#        [Unit]
#        Description=BREC
#        After=syslog.target network.target
#
#        [Service]
#        Type=forking
#        ExecStart=/home/root/brec-start.sh
#
#        [Install]
#        WantedBy=multi-user.target
#    2.3 Create a symlink to service definition
#        ln /lib/systemd/brec-start.service \
#           /etc/systemd/system/brec-start.service 
#    2.4 Reload/Restart system services
#        systemctl daemon-reload
#        systemctl start  brec-start.service
#        systemctl enable brec-start.service
#    2.5 Reboot the unit
#        shutdown -r now
#3.0 Usefull debugging commands
#    3.1 systemctl --all
#    3.2 systemctl status brec-start.service
#

###############################################################################
# 
# Start of Script
#
echo "BREC start beginning"

########################################
#
# Install device tree
#
/home/root/Bbb/Dt/dt-install

########################################
#
# Initial GPIO configurations
#
/home/root/Bbb/Scripts/g0 0
/home/root/Bbb/Scripts/g1 0
/home/root/Bbb/Scripts/fs-set 3

########################################
#
# Start workbench server
#
/home/root/Bbb/WbSvr/arm/WbSvr > /tmp/wbsvr.log  &

########################################
#
# Login to work bench and perform cfg
#
sleep 3
telnet localhost 6790 << EndOfCommands 

# Following are for debug
#wb-dbg 0xffff
#help

wb-syn-freq-hz    0 36000000
wb-syn-aux-enable 0 1
wb-syn-aux-power  0 0 

wb-syn-freq-hz    1 36000000
wb-syn-aux-enable 1 1
wb-syn-aux-power  1 0 

EndOfCommands

echo "BREC start complete"

# NOTE: This script must exit.  If time exceeded it and all children
# are terminated.

# Need to signal success so children are note terminated
exit 0
