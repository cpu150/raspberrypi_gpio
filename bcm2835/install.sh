#!/bin/bash

BCM2835="bcm2835-1.62"

echo "INSTALLING BCM2835 V1.62 LIBRARY TO HOME DIRECTORY..."
cd ~
wget http://www.airspayce.com/mikem/bcm2835/${BCM2835}.tar.gz
if [ -f "${BCM2835}.tar.gz" ]; then
	tar -zvxf ${BCM2835}.tar.gz
else
	echo "ERROR WHILE DOWNLOADING ${BCM2835} LIBRARY"
	exit -1
fi

if [ ! -d ${BCM2835} ]; then
	echo "ERROR WHILE UNCOMPRESSING ${BCM2835}.tar.gz file"
	exit -2
fi

echo "COMPILING LIBRARY..."
cd bcm2835-1.62
./configure
make

echo "INSTALLING LIBRARY..."
sudo make install

echo "DONE"
