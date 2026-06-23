CC = gcc

CFLAGS = -Wall -Wextra -g -I"../raylib" -I"../net-prog/paho.mqtt.c/src"
LDFLAGS = -L"../raylib" -L"../net-prog/paho.mqtt.c/src" -lraylib -lpthread -ldl -lrt -lX11 -l:libpaho-mqtt3a.a -lm

SRCS = test.c \
       ../src/engine/engine.c \
       ../src/engine/engine_context.c \
       ../src/menu.c \
       ../src/engine/texture_manager.c \
       ../src/config/config.c \
       ../src/config/cJSON.c \
       ../src/door.c \
       ../src/engine/console.c \
       ../src/map.c \
       ../src/player.c

all: test_entity_manager

test_entity_manager:
	$(CC) $(CFLAGS) $(SRCS) -o test_entity_manager `pkg-config --cflags --libs check` $(LDFLAGS)

clean:
	rm test_entity_manager