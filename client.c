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
        char *receivebuffer;
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

        receivebuffer = (char *) malloc(size);
        if (!receivebuffer) {
                perror("failed to allocated receivebuffer");
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
        // char measurebyte[65535];
        // char *mb = measurebyte;
        // *(unsigned short *) (mb) = (unsigned short) htons(65535);
        // memset(mb+2, 65, sizeof(measurebyte) - 2);

        char measurebyte = 'a';
        char *mb = &measurebyte;
        double independent_delay;
        char measureByte = 1;
        unsigned int long y1;
        double k;
        gettimeofday(&starttime, NULL);
        send(sock, &measureByte, 1, 0);
        count = recv(sock, endtime, sizeof(struct timeval), 0);
        gettimeofday(&starttime1, NULL);
        if (count < 0) {
                perror("receive failure");
                abort();
        } else {
//                endtime->tv_sec = ntohl(endtime->tv_sec);
//                endtime->tv_usec = ntohl(endtime->tv_usec);
                y1 = (starttime1.tv_sec - starttime.tv_sec) * 1000000L + (starttime1.tv_usec - starttime.tv_usec);
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
//                fwrite(sendbuffer+10, size-10, 1, stdout);
//                fprintf(stdout, "\n");

                int recvcount = 0;
                int sendcount = 0;
//                printf("sendcount before %d\n", sendcount);
                while (sendcount < size) {
                        sendcount += send(sock, sendbuffer + sendcount, size - sendcount, 0);
                }
//                printf("sendcount after %d\n", sendcount);

//                printf("recvcount before %d\n", recvcount);
                while (recvcount < size) {
                        recvcount += recv(sock, receivebuffer + recvcount, size - recvcount, 0);
                }
//                printf("recvcount after %d\n", recvcount);

                gettimeofday(&recvtime, NULL);
                measured_delay += ((recvtime.tv_sec - time.tv_sec) * 1000000L) + recvtime.tv_usec - time.tv_usec;
        }
        close(sock);
        free(sendbuffer);
        free(receivebuffer);
        //Calculate average latency
        measured_delay = measured_delay / COUNT;
        double average_latency = (double)measured_delay / 1000;
        measured_delay = 50;
        y1 = 60;
        size = 65535;
        k = (double)(measured_delay-y1) / (size-1);
        fprintf(stdout, "size: %d\n", size);
        fprintf(stdout, "y1: %ld\n", y1);
        fprintf(stdout, "measured_delay: %ld\n", measured_delay);
        fprintf(stdout, "k: %f\n", k);
        independent_delay = (double)y1-k;
        double measured_bandwidth = (1.0/k) / 1000000L;
        // Assume c to s is the same as s to c
//        unsigned int long measured_bandwidth = ((size * 2 * 8) * 1000000L / measured_delay) / 1000000L;
        printf("The independent delay is %0.3f milliseconds.\n", (double)independent_delay/1000);
        printf("The average latency of the exchanges is %0.3f milliseconds.\n", average_latency);
        printf("The measured bandwidth is %f Mbps. \n", measured_bandwidth);

        return 0;
}