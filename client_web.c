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
    int BUF_LEN = 65535;
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

    sendbuffer = (char *) malloc(BUF_LEN);
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
    int i;
    for (i = 0; i < COUNT; i++) {
        strcpy(sendbuffer, "GET /name_addr.c HTTP/1.1 \r\n\r\n");
        fprintf(stdout, "sendbuffer size: %d\n", strlen(sendbuffer));
        send(sock, sendbuffer, strlen(sendbuffer), 0);

        count = recv(sock, sendbuffer, BUF_LEN, 0);
        fprintf(stdout, "count: %d\n", count);
//        fprintf(stdout, "sendbuffer: %s\n", sendbuffer);
        fwrite(sendbuffer, count, 1, stdout);
    }
    close(sock);
    free(sendbuffer);
    return 0;
}
