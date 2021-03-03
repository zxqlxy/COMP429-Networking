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
    int i;

    /* allocate a memory buffer in the heap */
    /* putting a buffer on the stack like:
       leaves the potential for
       buffer overflow vulnerability */

    sendbuffer = (char *) malloc(65535);
    if (!sendbuffer) {
        perror("failed to allocated sendbuffer");
        abort();
    }

    receivebuffer = (char *) malloc(65535);
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

    // Send message
    struct timeval time, recvtime;
    double measured_delay = 0.0;
    char d[size-10];
    char *dp = d;

    memset(dp, 65 , sizeof(d));

    *(unsigned short *) (sendbuffer) = (unsigned short) htons(size);
    memcpy(sendbuffer+10, &d, sizeof(d));
    for (i = 0; i < COUNT; i++) {
        gettimeofday(&time, NULL);
        *(unsigned int *) (sendbuffer + 2) = (unsigned int) htonl(time.tv_sec);
        *(unsigned int *) (sendbuffer + 6) = (unsigned int) htonl(time.tv_usec);
        // Not necessary to print message

        int recvcount = 0;
        int sendcount = 0;

        while (sendcount < size) {
            sendcount += send(sock, sendbuffer + sendcount, size - sendcount, 0);
        }

        while (recvcount < size) {
            recvcount += recv(sock, receivebuffer + recvcount, size - recvcount, 0);
        }

        gettimeofday(&recvtime, NULL);
        measured_delay += ((recvtime.tv_sec - time.tv_sec) * 1000000L) + recvtime.tv_usec - time.tv_usec;
    }
    free(sendbuffer);
    free(receivebuffer);
    measured_delay = measured_delay / COUNT;
    double average_latency = measured_delay / 1000;
    printf("The average latency of the exchanges is %0.3f milliseconds.\n", average_latency);
    close(sock);
    return 0;
}