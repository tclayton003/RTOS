#!/bin/sh
rm -rv `find ./ -iname "iar"`
rm -rv applications
mv boards/dummy dummy
rm -rv boards/*
mv dummy boards/
rm -rv components/accelerometer/
rm -rv components/accelerometer/
rm -rv components/audio/
rm -rv components/clocks/
rm -rv components/display/
rm -rv components/ethernet_phy/
rm -rv components/joystick/
rm -rv components/touch/
rm -rv services/audio/
rm -rv services/dsp/
rm -rv services/freq_detect/
rm -rv services/fs/
rm -rv services/storage/
rm -rv services/usb/
rm -rv services/network/lin/
mv utils/debug/debug.h utils/debug/debug_asf.h 

