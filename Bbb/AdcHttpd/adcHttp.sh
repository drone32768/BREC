#!/bin/bash

export LD_LIBRARY_PATH='/usr/local/lib';

while true; do
   arm/AdcHttpd -port 8081 
done


