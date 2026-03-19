CC = gcc
TINYEMU_DIR = ./tinyemu-2019-12-21

CFLAGS := -Wall -g -I"./raylib"
LDFLAGS := -L"./raylib" -lraylib -lm -lpthread -ldl -lrt -lX11 -I"net-prog/paho.mqtt.c/src" -L"net-prog/paho.mqtt.c/src" -l:libpaho-mqtt3a.a

TARGET := main
SRCS := src/main.c src/console.c src/map.c src/player.c src/game.c src/menu.c src/door.c src/engien/engien.c

OBJS := $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	make -C $(TINYEMU_DIR)
	make -C ./raylib
	cp ./src/libraylib.a ./raylib
	$(CC) $(CFLAGS) -o $@ -g $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	make clean -C ./raylib
	rm -f ./raylib/libraylib.a
	rm -f $(OBJS) $(TARGET)
	make -C $(TINYEMU_DIR) clean

start:
	-sudo ip link delete tap0 2>/dev/null
	sudo ip tuntap add mode tap user $(USER) name tap0
	sudo ip addr add 192.168.3.1/24 dev tap0
	sudo ip link set tap0 up
	@echo "===Проверка tap0==="
	ip addr show tap0

	./$(TARGET)