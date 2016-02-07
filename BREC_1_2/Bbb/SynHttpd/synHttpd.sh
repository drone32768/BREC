#!/bin/bash


export LD_LIBRARY_PATH='/usr/local/lib';

while true; do
   arm/synHttpd -port 80 -config syn.cfg 
done

