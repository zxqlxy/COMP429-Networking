

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/* A linked list node data structure to maintain application
   information related to a connected socket */
struct window
{
    char *buffer;


    int socket;
    struct sockaddr_in client_addr;
    int pending_data; /* flag to indicate whether there is more data to send */
    /* you will need to introduce some variables here to record
       all the information regarding this socket.
       e.g. what data needs to be sent next */
    char *buf;
    int idx;
    int buf_len;
    int testByte;
    struct timeval *server_recv_start;
    struct timeval *server_recv_end;
    struct node *next;
};

int main(int argc, char **argv) {

        /*
         * Parse the information about the receiver
         */
        char *receiver_info = argv(2);
        char *recv_host;
        char *recv_port;
        recv_host = strtok(receiver_info, ':');
        recv_port = atoi(strtok(NULL, ':'));

        /*
         * Parse the information about the file to be sent
         */
        char *file_info = argv(4);
        char *subdir;
        char *filename;
        subdir = strtok(file_info, '/');
        filename = atoi(strtok(NULL, '/'));


        /* create the sender socket */
        int send_sock;
        if ((send_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("opening TCP socket");
                abort();
        }

        /*
         * Put information about the receiver in a struct.
         * Used in sendto and receiverfrom
         */
        struct sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = inet_addr(recv__host);
        sin.sin_port = htons(recv_port);

        /*
         * Open the file
         */
        FILE *fp = NULL;
        fp = fopen(file_info, 'r');

        // Not sure.
        int overhead;

        /*
         * Initialize packet
         */
        char *packet;
        packet = malloc((35000+overhead)*sizeof(char));
        char *buffer;


}
