#!/bin/bash

sudo ip link delete tap0 2>/dev/null
sudo ip tuntap add mode tap user $USER name tap0
sudo ip addr add 192.168.3.1/24 dev tap0

sudo ip link set tap0 up

echo "===Проверка tap0==="
ip addr show tap0

./tinyemu-2019-12-21/temu -ctrlc ./conf/root-riscv64.cfg
