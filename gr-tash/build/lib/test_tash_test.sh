#!/bin/sh
export VOLK_GENERIC=1
export GR_DONT_LOAD_PREFS=1
export srcdir=/home/zhenlin/Documents/RFID/BasicMoudle/gr-tash/lib
export PATH=/home/zhenlin/Documents/RFID/BasicMoudle/gr-tash/build/lib:$PATH
export LD_LIBRARY_PATH=/home/zhenlin/Documents/RFID/BasicMoudle/gr-tash/build/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$PYTHONPATH
test-tash 
