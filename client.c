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

/* simple client, takes two parameters, the server domain name,
   and the server port number */

int main(int argc, char **argv) {

        /* our client socket */
        int sock;

        /* variables for identifying the server */
        unsigned int server_addr;
        struct sockaddr_in sin;
        struct addrinfo *getaddrinfo_result, hints;

        /* convert server domain name to IP address */
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_INET; /* indicates we want IPv4 */

        if (getaddrinfo(argv[1], NULL, &hints, &getaddrinfo_result) == 0) {
                server_addr = (unsigned int) ((struct sockaddr_in *) (getaddrinfo_result->ai_addr))->sin_addr.s_addr;
                freeaddrinfo(getaddrinfo_result);
        }

        /* server port number */
        unsigned short server_port = atoi(argv[2]);
        unsigned short size = atoi(argv[3]);
        unsigned int COUNT = atoi(argv[4]);

        char *sendbuffer;
        int count;

        /* allocate a memory buffer in the heap */
        /* putting a buffer on the stack like:
           leaves the potential for
           buffer overflow vulnerability */

        sendbuffer = (char *) malloc(size);
        if (!sendbuffer) {
                perror("failed to allocated sendbuffer");
                abort();
        }

        /* create a socket */
        if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
                perror("opening TCP socket");
                abort();
        }

        /* fill in the server's address */
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = server_addr;
        sin.sin_port = htons(server_port);

        /* connect to the server */
        if (connect(sock, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
                perror("connect to server failed");
                abort();
        }


        // Measure the network link bandwidth
        struct timeval starttime;
        struct timeval starttime1;
        struct timeval *endtime;
        endtime = (struct timeval *) malloc(sizeof(struct timeval));
<<<<<<< HEAD
        // char measurebyte[65535];
        // char *mb = measurebyte;
        // *(unsigned short *) (mb) = (unsigned short) htons(65535);
        // memset(mb+2, 65, sizeof(measurebyte) - 2);
        char measurebyte = 'a';
        char *mb = &measurebyte;
=======
//        char measurebyte[65535];
//        char *mb = measurebyte;
//        *(unsigned short *) (mb) = (unsigned short) htons(65535);
//        memset(mb+2, 65, sizeof(measurebyte) - 2);
>>>>>>> 3dae9b8517223c9e9d0854bbd18a5cf8ce84e46e
        unsigned int long independent_delay;
        char measureByte = 1;
        gettimeofday(&starttime, NULL);
        send(sock, &measureByte, 1, 0);
        gettimeofday(&starttime1, NULL);
        count = recv(sock, endtime, sizeof(struct timeval), 0);
        if (count < 0) {
                perror("receive failure");
                abort();
        } else {
                endtime->tv_sec = ntohl(endtime->tv_sec);
                endtime->tv_usec = ntohl(endtime->tv_usec);
                independent_delay = ((endtime->tv_sec) * 1000000L + (endtime->tv_usec))*2 +
                        ((starttime1.tv_sec - starttime.tv_sec) * 1000000L + (starttime1.tv_usec - starttime.tv_usec)) * 2;
        }
        free(endtime);

        struct timeval time, recvtime;
        unsigned int long measured_delay = 0;
        // gettimeofday(&time, NULL);
        char d[size-10];
        char *dp = d;

        memset(dp, 65 , sizeof(d));

        *(unsigned short *) (sendbuffer) = (unsigned short) htons(size);
        // *(unsigned int *) (sendbuffer + 2) = (unsigned int) htonl(time.tv_sec);
        // *(unsigned int *) (sendbuffer + 6) = (unsigned int) htonl(time.tv_usec);
        memcpy(sendbuffer+10, &d, sizeof(d));
        int i;
        for (i = 0; i < COUNT; i++) {
                gettimeofday(&time, NULL);
                *(unsigned int *) (sendbuffer + 2) = (unsigned int) htonl(time.tv_sec);
                *(unsigned int *) (sendbuffer + 6) = (unsigned int) htonl(time.tv_usec);
                send(sock, sendbuffer, size, 0);
                count = recv(sock, sendbuffer, size, 0);
                if (count < 0) {
                        perror("receive failure");
                        abort();
                } else if (count < size) {
                        perror("not fully received");
                }
                fwrite(sendbuffer+10, size-10, 1, stdout);
                fprintf(stdout, "\n");
                gettimeofday(&recvtime, NULL);
                measured_delay += ((recvtime.tv_sec - time.tv_sec) * 1000000L) + recvtime.tv_usec - time.tv_usec;
        }
        close(sock);
        free(sendbuffer);
        //Calculate independent delay
        measured_delay = measured_delay / COUNT;
        // Assume c to s is the same as s to c
        unsigned int long measured_bandwidth = (size * 2 * 8) * 1000000L / measured_delay;
        printf("The independent delay is %ld microseconds.\n", independent_delay);
        printf("Measured delay is %ld microseconds.\n", measured_delay);
        printf("The measured bandwidth is %ld Mbps. \n", measured_bandwidth);

        return 0;
}
