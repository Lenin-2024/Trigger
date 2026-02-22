#!/bin/bash

# cd net-prog/

# echo "=== Компиляция door ==="
# make clean
# make

# cd ..

mkdir -p /tmp/rootfs

sudo mount -o loop ../conf/rootfs.ext2 /tmp/rootfs

sudo cp client  /tmp/rootfs/usr/bin/

echo ==Проверка копирования файлов==
ls -la /tmp/rootfs/usr/bin/ | grep "client"

sudo chmod +x /tmp/rootfs/usr/bin/client

sudo umount /tmp/rootfs

echo "Готово! Файл скопирован в rootfs.ext2"
