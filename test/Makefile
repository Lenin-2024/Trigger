CC = gcc

CFLAGS = -Wall -Wextra -g -I"../raylib" -I"../net-prog/paho.mqtt.c/src"
LDFLAGS = -L"../raylib" -L"../net-prog/paho.mqtt.c/src" -lraylib -lpthread -ldl -lrt -lX11 -l:libpaho-mqtt3a.a -lm

COMMON_SRCS = ../src/engine/engine.c \
              ../src/engine/engine_context.c \
              ../src/menu.c \
              ../src/engine/texture_manager.c \
              ../src/config/config.c \
              ../src/config/cJSON.c \
              ../src/door.c \
              ../src/engine/console.c \
              ../src/map.c \
              ../src/player.c

EM_SRCS = test.c $(COMMON_SRCS)
CONSOLE_SRCS = game_test.c $(COMMON_SRCS)

all: test_entity_manager test-console

test_entity_manager:
	$(CC) $(CFLAGS) $(EM_SRCS) -o test_entity_manager `pkg-config --cflags --libs check` $(LDFLAGS)

test-console:
	$(CC) $(CFLAGS) $(CONSOLE_SRCS) -o test-console `pkg-config --cflags --libs check` $(LDFLAGS)

clean:
	rm -f test_entity_manager test-console
