#!/bin/bash

cd net-prog/

echo "=== Компиляция door ==="
make clean
make

cd ..

mkdir -p /tmp/rootfs

sudo mount -o loop conf/rootfs.ext2 /tmp/rootfs

sudo cp net-prog/door/door  /tmp/rootfs/usr/bin/

echo ==Проверка копирования файлов==
ls -la /tmp/rootfs/usr/bin/ | grep "door"

sudo chmod +x /tmp/rootfs/usr/bin/door

sudo umount /tmp/rootfs

echo "Готово! Файл скопирован в rootfs.ext2"