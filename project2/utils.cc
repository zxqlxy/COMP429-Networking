#include "utils.h"

#include <iostream>
using namespace std;

char checksum(char *frame, int count) {
    u_long sum = 0;
    while (count--) {
        sum += *frame++;
        if (sum & 0xFFFF0000) {
            sum &= 0xFFFF;
            sum++; 
        }
    }
    return (sum & 0xFFFF);
}

/* 
 type 1: datapacket: int seq_num, char *data, int data_size, bool eot)
 type 2: congestion: int window_len
 tpye 3: subdir packet: int size, char *dir
 type 0: eot
 */
int create_frame(char type, char *frame, char *args[])  {
    if (type == 1){
        int seq_num = (int) atoi(args[0]);
        char* data = args[1];
        int data_size = (int) atoi(args[2]);
        bool eot = (bool) args[3];
        frame[0] = eot ? 0x0 : 0x1;
        uint32_t net_seq_num = htonl(seq_num);
        uint32_t net_data_size = htonl(data_size);
        memcpy(frame + 1, &net_seq_num, 4);
        memcpy(frame + 5, &net_data_size, 4);
        memcpy(frame + 9, data, data_size);
        frame[data_size + 9] = checksum(frame, data_size + (int) 9);
        return data_size + (int)10;
    } else if (type == 2) {
        frame[0] = 2;
        int window_len = htonl(atoi(args[0]));
        memcpy(frame + 1, &window_len, 4);
        // TODO Checksum
        return 5;
    } else if (type == 3) {
        frame[0] = 3;
        int data_size = htonl(atoi(args[0]));
        memcpy(frame + 1, &data_size, 4);
        char* data = args[1];
        memcpy(frame + 5, data, data_size);
        // TODO Checksum
        return 5 + data_size;
    }
    return 0;
}

void create_ack(char type, int seq_num, char *ack, bool error) {
    ack[0] = error ? 0x0 : (char) type;
    uint32_t net_seq_num = htonl(seq_num);
    memcpy(ack + 1, &net_seq_num, 4);
    ack[5] = checksum(ack, ACK_SIZE - (int) 1);
}

/* 
 type 1: datapacket: int *seq_num, char *data, int *data_size, bool *eot
 type 2: congestion: int window_len
 tpye 3: subdir packet: int size, char *dir
 type 0: eot
 */
bool read_frame(char *frame, char *argv[]) {
    char type = frame[0];
    if (type == 1 || type == 0) {
        int *seq_num = (int *) argv[0];
        char *data = argv[1];
        int *data_size = (int *) argv[2];
        bool *eot = (bool *) argv[3];
    
        *eot = type;

        uint32_t net_seq_num;
        memcpy(&net_seq_num, frame + 1, 4);
        *seq_num = ntohl(net_seq_num);

        uint32_t net_data_size;
        memcpy(&net_data_size, frame + 5, 4);
        *data_size = ntohl(net_data_size);

        memcpy(data, frame + 9, *data_size);

        return frame[*data_size + 9] != checksum(frame, *data_size + (int) 9);
    } else if (type == 2) {
        int *window_len = (int *) argv[0];
        int window;
        memcpy(&window, frame + 1, 4);
        *window_len = ntohl(window);
    } else if (type == 3) {
        int *data_size = (int *) argv[0];
        char *data = argv[1];
    }
}

bool read_ack(int *seq_num, bool *neg, char *ack) {
    *neg = ack[0] == 0x0 ? true : false;

    uint32_t net_seq_num;
    memcpy(&net_seq_num, ack + 1, 4);
    *seq_num = ntohl(net_seq_num);

    return ack[5] != checksum(ack, ACK_SIZE - (int) 1);
}
