CC	 	= gcc
LD	 	= gcc
CFLAGS	 	= -Wall -g

LDFLAGS	 	= 
DEFS 	 	=

all:	server client

server: server.c
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o server server.c

client: client.c
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o client client.c

clean:
	rm -f *.o
	rm -f *~
	rm -f core.*.*
	rm -f server
	rm -f client
