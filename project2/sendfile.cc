#include <iostream>
#include <thread>
#include <mutex>

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#include "utils.h"

#define TIMEOUT 10

using namespace std;

int socket_fd;
struct sockaddr_in server_addr, client_addr;

int window_len;
bool *window_ack_mask;
time_stamp *window_sent_time;
int lar, lfs;

time_stamp TMIN = current_time();
mutex window_info_mutex;
bool conjest = false;

void listen_ack() {
    char ack[ACK_SIZE];
    int ack_size;
    int ack_seq_num;
    bool ack_error;
    bool ack_neg;

    /* Listen for ack from reciever */
    while (true) {
        socklen_t server_addr_size;
        ack_size = recvfrom(socket_fd, (char *)ack, ACK_SIZE, 
                MSG_WAITALL, (struct sockaddr *) &server_addr, 
                &server_addr_size);
        ack_error = read_ack(&ack_seq_num, &ack_neg, ack);

        window_info_mutex.lock();

        if (!ack_error && ack_seq_num > lar && ack_seq_num <= lfs) {
            if (!ack_neg) {
                window_ack_mask[ack_seq_num - (lar + 1)] = true;
            } else {
                window_sent_time[ack_seq_num - (lar + 1)] = TMIN;
            }
        }

        window_info_mutex.unlock();
    }
}

int main(int argc, char *argv[]) {
    char *recv_host;
    int recv_port;
    int max_buffer_size;
    struct hostent *dest_hnet;
    char *file_info;

    if (argc == 5) {

        char *flag1 = argv[1], *flag2 = argv[3];
        int r, f;
        if (strcmp(flag1, "-r") == 0){
            r = 1;
        } else if (strcmp(flag1, "-f") == 0){
            f = 1;
        } else {
            cerr << "flag can only be -r or -f" << endl;
        }

        if (strcmp(flag2, "-r") == 0){
            r = 3;
        } else if (strcmp(flag2, "-f") == 0){
            f = 3;
        } else {
            cerr << "flag can only be -r or -f" << endl;
        }
        
        /*
         * Parse the information about the receiver
         */

        char *receiver_info = argv[r+1];
        char *recv_host;
        int recv_port;
        recv_host = strtok(receiver_info, ":");
        recv_port = atoi(strtok(NULL, ":"));
        cout << recv_host << "hhh" << recv_port << endl;

        /*
         * Parse the information about the file to be sent
         */
        file_info = argv[f+1];
        char *subdir;
        char *filename;
        cout << file_info << endl;
        // subdir = strtok(file_info, "/");
        // filename = strtok(NULL, "/");

        window_len = 100;
        max_buffer_size = MAX_DATA_SIZE * 1000;
    } else {
        cerr << "usage: ./sendfile -r <recv host>:<recv port> -f <subdir>/<filename>" << endl;
        return 1; 
    }

    /* Get hostnet from server hostname or IP address */ 
    dest_hnet = gethostbyname(recv_host); 
    if (!dest_hnet) {
        cerr << "unknown host: " << recv_host << endl;
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr)); 
    memset(&client_addr, 0, sizeof(client_addr)); 

    /* Fill server address data structure */
    server_addr.sin_family = AF_INET;
    bcopy(dest_hnet->h_addr, (char *)&server_addr.sin_addr, 
            dest_hnet->h_length); 
    server_addr.sin_port = htons(recv_port);

    /* Fill client address data structure */
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY; 
    client_addr.sin_port = htons(0);

    /* Create socket file descriptor */ 
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cerr << "socket creation failed" << endl;
        return 1;
    }

    /* Bind socket to client address */
    if (::bind(socket_fd, (const struct sockaddr *)&client_addr, 
            sizeof(client_addr)) < 0) { 
        cerr << "socket binding failed" << endl;
        return 1;
    }

    if (access(file_info, F_OK) == -1) {
        cerr << "file doesn't exist: " << file_info << endl;
        return 1;
    }

    /* Open file to send */
    FILE *file = fopen(file_info, "rb");
    char buffer[max_buffer_size];
    int buffer_size;

    /* Start thread to listen for ack */
    thread recv_thread(listen_ack);

    char frame[MAX_FRAME_SIZE];
    char data[MAX_DATA_SIZE];
    int frame_size;
    int data_size;

    /* Send file */
    bool read_done = false;
    int buffer_num = 0;
    while (!read_done) {

        /* Read part of file to buffer */
        buffer_size = fread(buffer, 1, max_buffer_size, file);
        if (buffer_size == max_buffer_size) {
            char temp[1];
            int next_buffer_size = fread(temp, 1, 1, file);
            if (next_buffer_size == 0) read_done = true;
            int error = fseek(file, -1, SEEK_CUR);
        } else if (buffer_size < max_buffer_size) {
            read_done = true;
        }
        
        window_info_mutex.lock();

        /* Initialize sliding window variables */
        int seq_count = buffer_size / MAX_DATA_SIZE + ((buffer_size % MAX_DATA_SIZE == 0) ? 0 : 1);
        int seq_num;
        window_sent_time = new time_stamp[MAX_WINDOW_SIZE];
        window_ack_mask = new bool[MAX_WINDOW_SIZE];
        bool window_sent_mask[MAX_WINDOW_SIZE];
        for (int i = 0; i < window_len; i++) {
            window_ack_mask[i] = false;
            window_sent_mask[i] = false;
        }
        lar = -1;
        lfs = lar + window_len;

        window_info_mutex.unlock();
        
        /* Send current buffer with sliding window */
        bool send_done = false;
        while (!send_done) {

            window_info_mutex.lock();

            /* Check window ack mask, shift window if possible */
            if (window_ack_mask[0]) {
                int shift = 1;
                for (int i = 1; i < window_len; i++) {
                    if (!window_ack_mask[i]) break;
                    shift += 1;
                }
                for (int i = 0; i < window_len - shift; i++) {
                    window_sent_mask[i] = window_sent_mask[i + shift];
                    window_ack_mask[i] = window_ack_mask[i + shift];
                    window_sent_time[i] = window_sent_time[i + shift];
                }
                for (int i = window_len - shift; i < window_len; i++) {
                    window_sent_mask[i] = false;
                    window_ack_mask[i] = false;
                }
                lar += shift;
                lfs = lar + window_len;
            }

            window_info_mutex.unlock();

            /* Send frames that has not been sent or has timed out */
            for (int i = 0; i < window_len; i ++) {
                seq_num = lar + i + 1;

                if (seq_num < seq_count) {
                    window_info_mutex.lock();

                    if (!window_ack_mask[i] && (elapsed_time(current_time(), window_sent_time[i]) > TIMEOUT)){
                        conjest = true;
                    }
                    if (!window_sent_mask[i] || (!window_ack_mask[i] && (elapsed_time(current_time(), window_sent_time[i]) > TIMEOUT))) {
                        int buffer_shift = seq_num * MAX_DATA_SIZE;
                        data_size = (buffer_size - buffer_shift < MAX_DATA_SIZE) ? (buffer_size - buffer_shift) : MAX_DATA_SIZE;
                        memcpy(data, buffer + buffer_shift, data_size);
                        
                        bool eot = (seq_num == seq_count - 1) && (read_done);
                        frame_size = create_frame(seq_num, frame, data, data_size, eot);

                        sendto(socket_fd, frame, frame_size, 0, 
                                (const struct sockaddr *) &server_addr, sizeof(server_addr));
                        window_sent_mask[i] = true;
                        window_sent_time[i] = current_time();
                    }

                    window_info_mutex.unlock();
                }
            }

            if (conjest){
                // decrease
                window_len /= 2;
                conjest = false;
            } else {
                // increase 
                if (window_len <= MAX_WINDOW_SIZE - 10)
                    window_len += 10;
            }
            cout << "window lendgth %d" << window_len << endl;

            /* Move to next buffer if all frames in current buffer has been acked */
            if (lar >= seq_count - 1) send_done = true;
        }
        
        


        cout << "\r" << "[SENT " << (unsigned long long) buffer_num * (unsigned long long) 
                max_buffer_size + (unsigned long long) buffer_size << " BYTES]" << flush;
        buffer_num += 1;
        if (read_done) break;
    }
    
    fclose(file);
    delete [] window_ack_mask;
    delete [] window_sent_time;
    recv_thread.detach();

    cout << "\nAll done :)" << endl;
    return 0;
}
