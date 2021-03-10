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

#define DATASIZE = 1024
#define PACKETSIZE = 1034 // TODO
#define BUFSIZE = 1024000
#define WINDOWSIZE = 20
#define SEQNUM = 40

/* A sliding window structure */
struct window
{
    bool ACK;
    struct timeval *recv_start;
    struct timeval *recv_end;
};

int main(int argc, char **argv) {

    /*
     * Parse the information about the receiver
     */
    char *receiver_info = argv(2);
    char *recv_host;
    int recv_port;
    recv_host = strtok(receiver_info, ':');
    recv_port = atoi(strtok(NULL, ':'));

    /*
     * Parse the information about the file to be sent
     */
    char *file_info = argv(4);
    char *subdir;
    char *filename;
    subdir = strtok(file_info, '/');
    filename = strtok(NULL, '/');


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
    sin.sin_addr.s_addr = inet_addr(recv_host);
    sin.sin_port = htons(recv_port);

    struct sockaddr_in c_sin;
    memset(&c_sin, 0, sizeof(c_sin));
    c_sin.sin_family = AF_INET;
    // TODO not sure
    c_sin.sin_addr.s_addr = INADDR_ANY;
    c_sin.sin_port = htons(0);
    if (bind(send_sock, (const struct sockaddr *)&c_sin, sizeof(c_sin)) < 0) {
        return -1;
    }

    struct window windows[WINDOWSIZE];

    /*
     * Open the file
     */
    FILE *fp = NULL;
    fp = fopen(file_info, 'r');
    size_t buf_size = 0
    char buf[DATASIZE];
    if (fp != null) {
        fseek(fp, 0, SEEK_END);
        int length = (int)ftell(fp);
        rewind(fp);
        int i = 0;
        while (i < length) {
            buf_size = fread(buf, sizeof(char), BUFSIZE, fp);
            int sequence_count = buf_size / DATASIZE;
            if (buf_size % BUFSIZE != 0)
                sequence_count++;
            int idx = 0;
//            for

            i += buf_size;
        }
    } else {
        perror("Error reading the file\n");
    }


    /*
     * Initialize packet
     */
    char *packet;
    packet = malloc((35000+overhead)*sizeof(char));
    char *buffer;


}
