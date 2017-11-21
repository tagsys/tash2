#!/bin/sh
export VOLK_GENERIC=1
export GR_DONT_LOAD_PREFS=1
export srcdir=/home/zhenlin/Documents/RFID/BasicMoudle/gr-tash/python
export PATH=/home/zhenlin/Documents/RFID/BasicMoudle/gr-tash/build/python:$PATH
export LD_LIBRARY_PATH=/home/zhenlin/Documents/RFID/BasicMoudle/gr-tash/build/lib:$LD_LIBRARY_PATH
export PYTHONPATH=/home/zhenlin/Documents/RFID/BasicMoudle/gr-tash/build/swig:$PYTHONPATH
/usr/bin/python2 /home/zhenlin/Documents/RFID/BasicMoudle/gr-tash/python/qa_decoder.py 
