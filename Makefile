CC	 	= gcc
LD	 	= gcc
CFLAGS	 	= -Wall -g

LDFLAGS	 	= 
DEFS 	 	=

all:	server client name_addr server_num client_num

server_num: server.c
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o server_num server_num.c

client_num: client_num.c
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o client_num client_num.c

name_addr:	name_addr.c
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o name_addr name_addr.c

server: server.c
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o server server.c

client: client.c
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o client client.c

clean:
	rm -f *.o
	rm -f *~
	rm -f core.*.*
	rm -f server_num
	rm -f client_num
	rm -f server
	rm -f client
	rm -f name_addr
