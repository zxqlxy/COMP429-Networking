#include <hardware.h>





int create_packet(int seq_num, char *frame, char *data, int data_size, bool eot) {
        frame[0] = eot ? 0x0 : 0x1;
        uint32_t net_seq_num = htonl(seq_num);
        uint32_t net_data_size = htonl(data_size);
        memcpy(frame + 1, &net_seq_num, 4);
        memcpy(frame + 5, &net_data_size, 4);
        memcpy(frame + 9, data, data_size);
        frame[data_size + 9] = checksum(frame, data_size + (int) 9);

        return data_size + (int)10;
}

