CC = riscv64-linux-gnu-gcc

DOOR_DIR = door
STEMP_DIR = stemp

DOOR_SRCS = $(DOOR_DIR)/door.c net.c
DOOR_OBJS = $(DOOR_DIR)/door.o net.o

all: door/door

door/door: $(DOOR_OBJS)
	$(CC) $^ -o $@

net.o: net.c
	$(CC) -c $< -o $@

clean:
	rm -f door/*.o *.o door/door