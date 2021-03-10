CC	 	= g++ -std=c++11
LD	 	= g++ 
CFLAGS	 	= -pthread

LDFLAGS	 	= 
DEFS 	 	=

all: recvfile sendfile

recvfile: recvfile.cc
	$(CC) $(DEFS) $(CFLAGS) $(LIB) recvfile.cc -o recvfile


sendfile: sendfile.cc
	$(CC) $(DEFS) $(CFLAGS) $(LIB) sendfile.cc -o sendfile


clean: recvfile sendfile
	rm -f recvfile sendfile