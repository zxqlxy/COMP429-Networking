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
#include <pthread.h>

#define DATASIZE = 1024
#define PACKETSIZE = 1034 // TODO
#define BUFSIZE = 1024000
#define WINDOWSIZE = 20
#define SEQNUM = 40

/* A sliding window structure */
struct packet {
        bool ACK;
        struct timeval *recv_start;
        bool isSent;
//    char data[];
};

pthread_mutex_t lock;

int main(int argc, char **argv) {
        char *recv_host;
        int recv_port;
        int max_buffer_size;
//    struct hostent *dest_hnet;
        char *file_info;
        char *receiver_info;

        if (argc == 5) {

                char *flag1 = argv[1], *flag2 = argv[3];
                int r, f;
                if (strcmp(flag1, "-r") == 0) {
                        r = 1;
                } else if (strcmp(flag1, "-f") == 0) {
                        f = 1;
                } else {
                        printf("flag can only be -r or -f\n");
                        abort();
                }

                if (strcmp(flag2, "-r") == 0) {
                        r = 3;
                } else if (strcmp(flag2, "-f") == 0) {
                        f = 3;
                } else {
                        printf("flag can only be -r or -f\n");
                        abort();
                }

                /*
                 * Parse the information about the receiver
                 */

                receiver_info = argv[r + 1];
                recv_host = strtok(receiver_info, ":");
                recv_port = atoi(strtok(NULL, ":"));

                /*
                 * Parse the information about the file to be sent
                 */
                file_info = argv[f + 1];
                char *subdir;
                char *filename;
                subdir = strtok(file_info, "/");
                filename = strtok(NULL, "/");

                window_len = 100;
                max_buffer_size = MAX_DATA_SIZE * 1000;
        } else {
                printf("usage: ./sendfile -r <recv host>:<recv port> -f <subdir>/<filename>\n");
                abort();
                return 1;
        }

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
        struct sockaddr_in recv_sin;
        memset(&server_sin, 0, sizeof(recv_sin));
        recv_sin.sin_family = AF_INET;
        recv_sin.sin_addr.s_addr = inet_addr(recv_host);
        recv_sin.sin_port = htons(recv_port);

        struct sockaddr_in send_sin;
        memset(&c_sin, 0, sizeof(send_sin));
        send_sin.sin_family = AF_INET;
        // TODO not sure
        send_sin.sin_addr.s_addr = INADDR_ANY;
        send_sin.sin_port = htons(0);

        if (bind(send_sock, (const struct sockaddr *) &send_sin, sizeof(send_sin)) < 0) {
                return -1;
        }

        struct packet packets[ WINDOWSIZE];

        /*
         * Open the file
         */
        FILE *fp = NULL;
        fp = fopen(file_info, 'rb');
        size_t buf_size = 0;
        char buf[ BUFSIZE];
        int last_ack_received, last_packet_sent;
        if (fp != null) {
                bool read = true;
                while (read) {
                        pthread_mutex_lock(&lock);
                        buf_size = fread(buf, sizeof(char), BUFSIZE, fp);
                        if (buf_size < BUFSIZE)
                                read = false;
                        int sequence_count = buf_size / DATASIZE;
                        if (buf_size % DATASIZE != 0)
                                sequence_count++;

                        // Initialize sliding window
                        for (int i = 0; i < WINDOWSIZE; i++) {
                                packets[i].ACK = false;
                                packets[i].isSent = false;
//                memcpy(packets[i].data, buf+i*DATASIZE, DATASIZE);
                        }
                        // Initialize last ack received and last frame sent
                        last_ack_received = -1;
                        last_packet_sent = last_ack_received + WINDOWSIZE;
                        pthread_mutex_unlock(&lock);

                        bool send = true;
                        while (send) {
                                pthread_mutex_lock(&lock);
                                int shift;
                                /* Check ack, and shift if possible */
                                if (packets[0].ACK) {
                                        int i = 1;
                                        shift = 1;
                                        while (packets[i].ACK & i < WINDOWSIZE) {
                                                i++;
                                                shift += i;
                                        }
                                        


                                }
                                pthread_mutex_unlock(&lock);
                        }
                }
        } else {
                perror("Error reading the file\n");
        }


}