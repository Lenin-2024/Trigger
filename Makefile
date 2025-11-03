CC = gcc
TINYEMU_DIR = ./tinyemu-2019-12-21

all: main

main: main.o
	make -C $(TINYEMU_DIR)
	$(CC) main.o -o main

main.o: main.c
	$(CC) -c $< -o $@

clean:
	rm -f main.o main
	make -C $(TINYEMU_DIR) clean