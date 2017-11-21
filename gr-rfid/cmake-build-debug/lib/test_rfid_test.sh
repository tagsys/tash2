#!/bin/sh
export VOLK_GENERIC=1
export GR_DONT_LOAD_PREFS=1
export srcdir=/home/zhenlin/Documents/RFID/BasicMoudle/Tash/gr-rfid/lib
export GR_CONF_CONTROLPORT_ON=False
export PATH=/home/zhenlin/Documents/RFID/BasicMoudle/Tash/gr-rfid/cmake-build-debug/lib:$PATH
export LD_LIBRARY_PATH=/home/zhenlin/Documents/RFID/BasicMoudle/Tash/gr-rfid/cmake-build-debug/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$PYTHONPATH
test-rfid 
