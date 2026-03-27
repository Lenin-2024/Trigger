CC = riscv64-linux-gnu-gcc
CFLAGS = -Wall -static
LIBS = -I"paho.mqtt.c/src" -L"paho.mqtt.c/src" -l:libpaho-mqtt3c.a
PAHO_DIR = paho.mqtt.c

all: riscv serv client copy-prog

# Сборка библиотеки Paho MQTT для RISC-V
riscv:
	@echo "Building Paho MQTT library for RISC-V..."
	cd $(PAHO_DIR) && rm -rf CMakeCache.txt CMakeFiles
	cd $(PAHO_DIR) && cmake \
		-DCMAKE_C_COMPILER=/usr/bin/riscv64-linux-gnu-gcc \
		-DCMAKE_AR=/usr/bin/riscv64-linux-gnu-ar \
		-DCMAKE_RANLIB=/usr/bin/riscv64-linux-gnu-ranlib \
		-DPAHO_WITH_SSL=FALSE \
		-DPAHO_BUILD_DOCUMENTATION=FALSE \
		-DPAHO_BUILD_SAMPLES=FALSE \
		-DPAHO_BUILD_STATIC=TRUE \
		-DPAHO_BUILD_SHARED=FALSE
	cd $(PAHO_DIR) && make clean
	cd $(PAHO_DIR) && make
	@echo "[ INFO ] Paho MQTT library build completed successfully"

# Сборка библиотеки Paho MQTT для обычной архитектуры (CISC/x86)
cisc:
	@echo "Building Paho MQTT library for CISC/x86..."
	cd $(PAHO_DIR) && rm -rf CMakeCache.txt CMakeFiles
	cd $(PAHO_DIR) && cmake \
		-DPAHO_WITH_SSL=FALSE \
		-DPAHO_BUILD_DOCUMENTATION=FALSE \
		-DPAHO_BUILD_SAMPLES=FALSE \
		-DPAHO_BUILD_STATIC=TRUE \
		-DPAHO_BUILD_SHARED=FALSE
	cd $(PAHO_DIR) && make clean
	cd $(PAHO_DIR) && make
	@echo "[ INFO ] Paho MQTT library build completed successfully"

serv: src/serv.c
	$(CC) $(CFLAGS) -o serv src/serv.c $(LIBS)

client: src/client.c
	$(CC) $(CFLAGS) -o client src/client.c $(LIBS)

copy-prog: client
	mkdir -p /tmp/rootfs
	sudo mount -o loop ../conf/rootfs.ext2 /tmp/rootfs
	sudo cp client  /tmp/rootfs/usr/bin/

	@echo ==Проверка копирования файлов==
	ls -la /tmp/rootfs/usr/bin/ | grep "client" || (sudo umount /tmp/rootfs && echo "Ошибка: файл не найден!" && exit 1)
	
	sudo chmod +x /tmp/rootfs/usr/bin/client
	sudo umount /tmp/rootfs
	echo "Готово! Файл скопирован в rootfs.ext2"

help:
	@echo "Доступные цели:"
	@echo "  make           - Собрать для RISC-V (по умолчанию)"
	@echo "  make riscv     - Собрать для RISC-V"
	@echo "  make cisc      - Собрать для RISC-V"
	@echo "  make clean     - Очистить бинарники"
	@echo "  make help      - Показать эту справку"
	@echo "  make copy-prog - Скопировать программу"

clean:
	rm -f client serv
