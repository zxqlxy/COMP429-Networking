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
        // unsigned int COUNT = 5;

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
//        int num;

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

        /* everything looks good, since we are expecting a
           message from the server in this example, let's try receiving a
           message from the socket. this call will block until some data
           has been received */
        // count = recv(sock, buffer, size, 0);
        // if (count < 0) {
        //         perror("receive failure");
        //         abort();
        // }

//        /* in this simple example, the message is a string,
//           we expect the last byte of the string to be 0, i.e. end of string */
//        if (buffer[count - 1] != 0) {
//                /* In general, TCP recv can return any number of bytes, not
//               necessarily forming a complete message, so you need to
//               parse the input to see if a complete message has been received.
//                   if not, more calls to recv is needed to get a complete message.
//                */
//                printf("Message incomplete, something is still being transmitted\n");
//        } else {
//                printf("Here is what we got: %s", buffer);
//        }

//        while (1) {
//                printf("\nEnter the type of the number to send (options are char, short, int, or bye to quit): ");
//                fgets(buffer, size, stdin);
//                if (strncmp(buffer, "bye", 3) == 0) {
//                        /* free the resources, generally important! */
//                        close(sock);
//                        free(buffer);
//                        free(sendbuffer);
//                        return 0;
//                }
//
//                /* first byte of the sendbuffer is used to describe the number of
//                   bytes used to encode a number, the number value follows the first
//                   byte */
//                if (strncmp(buffer, "char", 4) == 0) {
//                        sendbuffer[0] = 1;
//                } else if (strncmp(buffer, "short", 5) == 0) {
//                        sendbuffer[0] = 2;
//                } else if (strncmp(buffer, "int", 3) == 0) {
//                        sendbuffer[0] = 4;
//                } else {
//                        printf("Invalid number type entered, %s\n", buffer);
//                        continue;
//                }
//
//                printf("Enter the value of the number to send: ");
//                fgets(buffer, size, stdin);
//                num = atol(buffer);
//
//                switch (sendbuffer[0]) {
//                        case 1:
//                                *(char *) (sendbuffer + 1) = (char) num;
//                                break;
//                        case 2:
//                                /* for 16 bit integer type, byte ordering matters */
//                                *(short *) (sendbuffer + 1) = (short) htons(num);
//                                break;
//                        case 4:
//                                /* for 32 bit integer type, byte ordering matters */
//                                *(int *) (sendbuffer + 1) = (int) htonl(num);
//                                break;
//                        default:
//                                break;
//                }
//                send(sock, sendbuffer, sendbuffer[0] + 1, 0);
//                count = recv(sock, buffer, size, 0);
//                if (count < 0) {
//                        perror("receive failure");
//                        abort();
//                } else {
//                        int num1 = (char) *(char *) (buffer + 1);
//                        printf("Receive from server %d \n", num1);
//                }
//        }


        //Measure the network link bandwidth
        struct timeval starttime;
        struct timeval starttime1;
        struct timeval *endtime;
        endtime = (struct timeval *) malloc(sizeof(struct timeval));
        // char *endtime;
        // endtime = (char *) malloc(8);
        // unsigned int end_sec, end_usec;
        char measurebyte = 'a';
        unsigned int long independent_delay;



//        starttime = (struct timeval *) malloc(sizeof(struct timeval));
//        starttime1 = (struct timeval *) malloc(sizeof(struct timeval));

        gettimeofday(&starttime, NULL);
        send(sock, &measurebyte, sizeof(measurebyte), 0);
        gettimeofday(&starttime1, NULL);
        count = recv(sock, endtime, sizeof(struct timeval), 0);
        if (count < 0) {
                perror("receive failure");
                abort();
        } else {
                endtime->tv_sec = ntohl(endtime->tv_sec);
                endtime->tv_usec = ntohl(endtime->tv_usec);
                // end_sec = ntohl((unsigned int) endtime);
                // end_usec = ntohl((unsigned int) (endtime + 4));
                independent_delay = ((endtime->tv_sec) * 1000000L + (endtime->tv_usec))*2 +
                        ((starttime1.tv_sec - starttime1.tv_sec) * 1000000L + (starttime1.tv_usec - starttime1.tv_usec)) * 2;
        }
        free(endtime);





        struct timeval time, recvtime;
        unsigned int long measured_delay = 0;
        // gettimeofday(&time, NULL);
        char d[size-10];
        char *dp = d;

        memset(dp, 65 , sizeof(d) + 1);

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
                fwrite(sendbuffer+10, size-10, 1, stdout);
                fprintf(stdout, "\n");
                count = recv(sock, sendbuffer, size, 0);
               if (count < 0) {
                       perror("receive failure");
                       abort();
               }

                // TODO: Measure
                gettimeofday(&recvtime, NULL);
                measured_delay += ((recvtime.tv_sec - time.tv_sec) * 1000000L) + recvtime.tv_usec - time.tv_usec;
        }
        close(sock);
        free(sendbuffer);
        //Calculate independent delay
        measured_delay = measured_delay / COUNT;
        // Assume c to s is the same as s to c
        unsigned int long measured_bandwidth = (size * 2 * 8) / measured_delay;
        printf("The independent delay is %ld microseconds.\n", independent_delay);
        printf("Measured delay is %ld microseconds.\n", measured_delay);
        printf("The measured bandwidth is %ld Mbps. \n", measured_bandwidth * 1000000L);

        return 0;
}
