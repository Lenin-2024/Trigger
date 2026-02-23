CC = gcc
CFLAGS = -Wall -static
LIBS = -I"paho.mqtt.c/src" -L"paho.mqtt.c/src" -l:libpaho-mqtt3c.a

all: serv client

serv: src/serv.c
	$(CC) $(CFLAGS) -o serv src/serv.c $(LIBS)

client: src/client.c
	$(CC) $(CFLAGS) -o client src/client.c $(LIBS)

clean:
	rm -f client serv
