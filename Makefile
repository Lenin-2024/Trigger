CC = gcc
TINYEMU_DIR = ./tinyemu-2019-12-21

CFLAGS := -Wall -g -I"./raylib"
LDFLAGS := -L"./raylib" -lraylib -lm -lpthread -ldl -lrt -lX11

TARGET := main
SRCS := src/main.c

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