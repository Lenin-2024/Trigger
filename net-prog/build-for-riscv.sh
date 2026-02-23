#!/bin/bash

cd paho.mqtt.c/

./make-build.sh

cd ..

make clean
make CC=riscv64-linux-gnu-gcc
