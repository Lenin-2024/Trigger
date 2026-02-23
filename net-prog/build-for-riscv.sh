#!/bin/bash

cd paho.mqtt.c/

./make-build.sh riscv

cd ..

make clean
make CC=riscv64-linux-gnu-gcc
