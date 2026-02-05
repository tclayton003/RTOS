#!/bin/bash

I2C=0
UART="/dev/ttyUSB1"
CAN="can0"

for i in "$@"
do
case $i in
    -i=*|--csp_i2c=*)
    I2C="${i#*=}"
    shift # past argument=value
    ;;
    -c=*|--csp-can=*)
    CAN="${i#*=}"
    shift # past argument=value
    ;;
    -k=*|--csp-kiss=*)
    UART="${i#*=}"
    shift # past argument=value
    ;;
    *)
    echo "  -c, --csp-can[=DEVICE] Add CAN interface, DEVICE=can0
  -i, --csp-i2c[=DEVICE] Add I2C interface, DEVICE=0
  -k, --csp-kiss[=DEVICE] Add KISS over UART interface, DEVICE=/dev/ttyUSB0"
    exit 0
    ;;
esac
done

./build/csp-ifc-test -i$I2C -a10
if [ $? != 0 ]
then
    echo "I2C (main) test failed"
else
    echo "I2C (main) test OK"
fi

./build/csp-ifc-test -k$UART -a15
if [ $? != 0 ]
then
    echo "UART test failed"
else
    echo "UART test OK"
fi

./build/csp-ifc-test -c$CAN -a20
if [ $? != 0 ]
then
    echo "CAN test failed"
else
    echo "CAN test OK"
fi

python set_aardvark_slave.py