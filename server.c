#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/time.h>


/**************************************************/
/* a few simple linked list functions             */
/**************************************************/

FILE * create_response(char *buf, char *result, FILE *fp, char *root_dir);

#define MAXLINE 2048

/* A linked list node data structure to maintain application
   information related to a connected socket */
struct node
{
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

/* remove the data structure associated with a connected socket
   used when tearing down the connection */
void dump(struct node *head, int socket)
{
    struct node *current, *temp;

    current = head;

    while (current->next)
    {
        if (current->next->socket == socket)
        {
            /* remove */
            temp = current->next;
            current->next = temp->next;
            free(temp); /* don't forget to free memory */
            return;
        }
        else
        {
            current = current->next;
        }
    }
}

/* create the data structure associated with a connected socket */
void add(struct node *head, int socket, struct sockaddr_in addr)
{
    struct node *new_node;

    new_node = (struct node *)malloc(sizeof(struct node));
    new_node->socket = socket;
    new_node->client_addr = addr;
    new_node->pending_data = 0;
    new_node->next = head->next;
    new_node->idx = 0;
    new_node->buf_len = 0;
    new_node->buf = (char *)malloc(65535);
    new_node->testByte = 0;
    new_node->server_recv_start = (struct timeval *)malloc(sizeof(struct timeval));
    new_node->server_recv_end = (struct timeval *)malloc(sizeof(struct timeval));
    head->next = new_node;
}

/*****************************************/
/* main program                          */
/*****************************************/

/* simple server, takes one parameter, the server port number */
int main(int argc, char **argv)
{

    /* socket and option variables */
    int sock, new_sock, max;
    int optval = 1;
    /* flag of www mode */
    int flag = 0;

    /* server socket address variables */
    struct sockaddr_in sin, addr;
    unsigned short server_port = atoi(argv[1]);

    /* socket address variables for a connected client */
    socklen_t addr_len = sizeof(struct sockaddr_in);

    /* maximum number of pending connection requests */
    int BACKLOG = 5;

    /* variables for select */
    fd_set read_set, write_set;
    struct timeval time_out;
    int select_retval;

    char *www = "www";
    char *mode = argv[2];
    char *root_dir = NULL;

    if (argc > 2) {
        //Is in web server mode
        if (strcmp(www, mode) == 0)
            flag = 1;

        if (flag == 1){
            root_dir = argv[3];
            fprintf(stdout, "enter %s mode, at %s directory\n", mode, root_dir);
        }

    }

    /* number of bytes sent/received */
    int size;

    /* linked list for keeping track of connected sockets */
    struct node head;
    struct node *current, *next;

    /* a buffer to read data */
    char *buf, *result;
    int BUF_LEN = 65535;

    buf = (char *)malloc(BUF_LEN);
    result = (char *)malloc(BUF_LEN);

    /* initialize dummy head node of linked list */
    head.socket = -1;
    head.next = 0;

    /* create a server socket to listen for TCP connection requests */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("opening TCP socket");
        abort();
    }

    /* set option so we can reuse the port number quickly after a restart */
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        perror("setting TCP socket option");
        abort();
    }

    /* fill in the address of the server socket */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(server_port);

    /* bind server socket to the address */
    if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        perror("binding socket to address");
        abort();
    }

    /* put the server socket in listen mode */
    if (listen(sock, BACKLOG) < 0)
    {
        perror("listen on socket failed");
        abort();
    }

    /* now we keep waiting for incoming connections,
       check for incoming data to receive,
       check for ready socket to send more data */
    while (1)
    {

        /* set up the file descriptor bit map that select should be watching */
        FD_ZERO(&read_set);  /* clear everything */
        FD_ZERO(&write_set); /* clear everything */

        FD_SET(sock, &read_set); /* put the listening socket in */
        max = sock;              /* initialize max */

        /* put connected sockets into the read and write sets to monitor them */
        for (current = head.next; current; current = current->next)
        {
            FD_SET(current->socket, &read_set);

            if (current->pending_data)
            {
                /* there is data pending to be sent, monitor the socket
                       in the write set so we know when it is ready to take more
                       data */
                FD_SET(current->socket, &write_set);
            }

            if (current->socket > max)
            {
                /* update max if necessary */
                max = current->socket;
            }
        }

        time_out.tv_usec = 100000; /* 1-tenth of a second timeout */
        time_out.tv_sec = 0;

        /* invoke select, make sure to pass max+1 !!! */
        select_retval = select(max + 1, &read_set, &write_set, NULL, &time_out);
        if (select_retval < 0)
        {
            perror("select failed");
            abort();
        }

        if (select_retval == 0)
        {
            /* no descriptor ready, timeout happened */
            continue;
        }

        if (select_retval > 0) /* at least one file descriptor is ready */
        {
            if (FD_ISSET(sock, &read_set)) /* check the server socket */
            {
                /* there is an incoming connection, try to accept it */
                new_sock = accept(sock, (struct sockaddr *)&addr, &addr_len);

                if (new_sock < 0)
                {
                    perror("error accepting connection");
                    abort();
                }

                /* make the socket non-blocking so send and recv will
                       return immediately if the socket is not ready.
                       this is important to ensure the server does not get
                       stuck when trying to send data to a socket that
                       has too much data to send already.
                     */
                if (fcntl(new_sock, F_SETFL, O_NONBLOCK) < 0)
                {
                    perror("making socket non-blocking");
                    abort();
                }

                /* the connection is made, everything is ready */
                /* let's see who's connecting to us */
                printf("Accepted connection. Client IP address is: %s\n",
                       inet_ntoa(addr.sin_addr));

                /* remember this client connection in our linked list */
                add(&head, new_sock, addr);
            }

            /* check other connected sockets, see if there is
                   anything to read or some socket is ready to send
                   more pending data */
            for (current = head.next; current; current = next)
            {
                next = current->next;
                struct timeval server_recv_end = *(current->server_recv_end);
                struct timeval server_recv_start = *(current->server_recv_start);
                /* see if we can now do some previously unsuccessful writes */
                if (FD_ISSET(current->socket, &write_set))
                {
//                    /* the socket is now ready to take more data */
//                    /* the socket data structure should have information
//                           describing what data is supposed to be sent next.
//                       but here for simplicity, let's say we are just
//                           sending whatever is in the buffer buf
//                         */
//                    size = send(current->socket, buf, BUF_LEN, MSG_DONTWAIT);
//                    if (size < 0)
//                    {
//                        if (errno == EAGAIN)
//                        {
//                            /* we are trying to dump too much data down the socket,
//                               it cannot take more for the time being
//                               will have to go back to select and wait til select
//                               tells us the socket is ready for writing
//                            */
//                        }
//                        else
//                        {
//                            /* something else is wrong */
//                        }
//                    }
//                    /* note that it is important to check size for exactly
//                           how many bytes were actually sent even when there are
//                           no error. send() may send only a portion of the buffer
//                           to be sent.
//                    */
                    if (current->testByte == 1) {

                        // This message is used to measure ideal bandwidth
                        int sendsize = send(current->socket, current->buf, 1, MSG_DONTWAIT);
                        if (sendsize < 0) {
                            printf("Send failed");
                        }
                        current->testByte = 0;
                        current->pending_data = 0;
                    } else {
//                                                                fwrite(buf + 10, size - 10, 1, stdout);
//                                                                fprintf(stdout, "\n");

//                        printf("write set prior to send, current buflen: %d, idx: %d, size: %d\n", current->buf_len, current->idx, size);
                        size = send(current->socket, current->buf + current->idx, current->buf_len-current->idx, MSG_DONTWAIT);

                        if (size <= 0)
                        {
                            /* something is wrong */
                            if (size == 0)
                            {
                                printf("Client closed connection. Client IP address is: %s\n",
                                       inet_ntoa(
                                               current->client_addr.sin_addr));
                            }
                            else
                            {
                                perror("error receiving from a client");
                            }

                            /* connection is closed, clean up */
                            close(current->socket);
                            dump(&head, current->socket);
                        }
                        else
                        {
                            current->idx = size;
                            if (current->idx == current->buf_len) {
                                current->idx = 0;
                                current->buf_len = 0;
                                current->pending_data = 0;
                            }
                            // printf("Successfully sent back message\n");
                        }
//                        printf("write set current buflen: %d, idx: %d, size: %d\n", current->buf_len, current->idx, size);
                    }
                }

                if (FD_ISSET(current->socket, &read_set))
                {
                    /* we have data from a client */

                    if (flag == 1)
                    {

                        FILE *fp = NULL;
                        size_t newLen = 0;

                        size = recv(current->socket, current->buf, BUF_LEN, 0);
                        if (size <= 0)
                        {

                            /* something is wrong */
                            if (size == 0)
                            {
                                printf("Client closed connection. Client IP address is: %s\n",
                                       inet_ntoa(current->client_addr.sin_addr));
                            }
                            else
                            {
                                perror("error receiving from a client");
                            }

                            /* connection is closed, clean up */
                            close(current->socket);
                            dump(&head, current->socket);
                        } else {

                            fp = create_response(current->buf, result, fp, root_dir);
                            if (fp == NULL) {
                                newLen = strlen(result);
                                fprintf(stdout, "FP is NULL\n");
                            }
                            if (fp != NULL) {
                                fprintf(stdout, "FP is not NULL\n");
                                fseek(fp, 0, SEEK_END);
                                int length = (int)ftell(fp);
                                rewind(fp);
                                newLen = fread(result + strlen(result), sizeof(char), length, fp)+strlen(result);
                                if ( ferror( fp ) != 0 ) {
                                    fputs("Error reading file", stderr);
                                } else {
                                    result[newLen++] = '\0'; /* Just to be safe. */
                                }
                                fclose(fp);
                            }

                            size = send(new_sock, result, newLen+1, 0);
                            if (size <= 0) {
                                /* something is wrong */
                                if (size == 0) {
                                    printf("Client closed connection. Client IP address is: %s\n",
                                           inet_ntoa(current->client_addr.sin_addr));
                                } else {
                                    perror("error receiving from a client");
                                }
                                /* connection is closed, clean up */
                                close(current->socket);
                                dump(&head, current->socket);
                            } else {
                                printf("Successfully sent the file\n");
                            }
                        }
                    }
                    else
                    {
//                        printf("enter read set\n");
//                        gettimeofday(&server_recv_start, NULL);
//                                                size = recv(current->socket, buf, BUF_LEN, MSG_DONTWAIT);
                        size = recv(current->socket, current->buf + current->idx, BUF_LEN-current->idx, MSG_DONTWAIT);
//                        printf("size: %d\n", size);
                        if (size == 1) {
                            current->testByte = 1;
                            current->pending_data = 1;
                        }
                        else if (size > 1) {
                            if (current->buf_len == 0) {
                                current->buf_len = ntohs(*(unsigned short *)(current->buf));
                            }
                            current->idx += size;
//                            printf("read set current buf_len: %d, idx: %d, size: %d\n", current->buf_len, current->idx, size);
                            if (current->idx == current->buf_len) {
                                current->idx = 0;
                                current->pending_data = 1;
                            }
                        } else {
                            if (errno == EAGAIN || errno == EWOULDBLOCK)
                            {
                                /* we are trying to dump too much data down the socket,
                                   it cannot take more for the time being
                                   will have to go back to select and wait til select
                                   tells us the socket is ready for writing
                                */
                                printf("EAGAIN error\n");
                            }
                                /* something is wrong */
                            else if (size == 0)
                            {
                                printf("Client closed connection. Client IP address is: %s\n",
                                       inet_ntoa(current->client_addr.sin_addr));
                            }
                            else
                            {
                                perror("error receiving from a client");
                            }

                            /* connection is closed, clean up */
                            close(current->socket);
                            dump(&head, current->socket);
                        }
//                        gettimeofday(&server_recv_end, NULL);
                    }
                }
            }
        }
    }
}

FILE*
create_response(char *buf, char *result, FILE *fp, char *root_dir)
{

    char method[MAXLINE], uri[MAXLINE], version[MAXLINE];

    int bad = 0;
    int not_found = 0;
    char *res;
    char *bad_res = "400 Bad Request";
    char *not_res = "404 Not Found";
    char *ok_res = "200 OK";
    char file_name[1024];

    /* Split into method, uri, version */
    sscanf(buf, "%s %s %s", method, uri, version);
    strcpy(file_name, root_dir);
    strcat(file_name, uri);

    /* Split uri into hostname/path */
    if (strncmp(method, "GET", strlen("GET")) != 0)
    {
        fprintf(stderr, "Only accept GET request!\n");
        bad = 1;
    }
    else if (strncmp(uri, "../", 3) == 0)
    {
        fprintf(stderr, "Cannot go above root directory\n");
        bad = 1;
    }
    else if (strncmp(version, "HTTP/1.1", strlen("HTTP/1.1")) != 0)
    {
        fprintf(stderr, "Wrong protocal\n");
        bad = 1;
    }

    if (bad != 1)
    {
        fprintf(stderr, "%s\n", file_name);
        fp = fopen(file_name, "r+");

        if (fp == NULL)
        {
            perror("Failed to the open file");
            not_found = 1;
        }
    }
    if (bad == 1)
        res = bad_res;
    else if (not_found == 1)
        res = not_res;
    else
        res = ok_res;
    /* Fill into result array */
    sprintf(result, "%s", version);
    sprintf(result, "%s %s\r\n", result, res);
    sprintf(result, "%sContent-Type: text/html\r\n", result);
    fprintf(stdout, "create response: \n%s \n", result);
    return fp;
}