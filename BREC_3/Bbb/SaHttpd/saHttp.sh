#!/bin/bash

export LD_LIBRARY_PATH='/usr/local/lib';

while true; do
   arm/SaHttpd -port 8081 
   sleep;
done


