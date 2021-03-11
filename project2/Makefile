CC	 	= g++ -std=c++11
LD	 	= g++ 
CFLAGS	 	= -pthread

LDFLAGS	 	= 
DEFS 	 	=

all: recvfile sendfile

recvfile: utils.h utils.cc recvfile.cc
	$(CC) $(DEFS) $(CFLAGS) $(LIB) utils.cc recvfile.cc -o recvfile


sendfile: utils.h utils.cc sendfile.cc
	$(CC) $(DEFS) $(CFLAGS) $(LIB) utils.cc sendfile.cc -o sendfile


clean: recvfile sendfile
	rm -f recvfile sendfile