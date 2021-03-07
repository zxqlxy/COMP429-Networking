

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
         * Open he file
         */
        FILE *fp = NULL;
        fp = fopen(file_info, 'r');


        /*
         * Initialize packet
         */
        char *packet;



}
