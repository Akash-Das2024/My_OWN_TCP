#ifndef MYSOCKET_H
#define MYSOCKET_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define MAX_MESSAGE_LENGTH 5000
#define MAX_SEND_BUFFER_SIZE 1000
#define SOCK_MyTCP 1001

// Structure to store a message
struct Message {
    char data[MAX_MESSAGE_LENGTH];
    int length;
};

// Structure to store the send buffer
struct Send_Buffer {
    struct Message messages[MAX_SEND_BUFFER_SIZE];
    int front, rear;
};

// Structure to store the received messages buffer
struct Received_Buffer {
    struct Message messages[MAX_SEND_BUFFER_SIZE];
    int front, rear;
};

// Structure to store the MyTCP socket data
struct MyTCP_Socket {
    int sockfd;
    struct Send_Buffer send_buffer;
    struct Received_Buffer received_buffer;
    bool closed;
    pthread_t send_thread;
    pthread_t receive_thread;
    pthread_mutex_t lock;
};

// Function to create a MyTCP socket
struct MyTCP_Socket * my_socket(int domain, int type, int protocol);

int my_bind(struct MyTCP_Socket *sock, const char* address, int port);
int my_listen(struct MyTCP_Socket *sock, int backlog);
int my_connect(struct MyTCP_Socket * sock, const struct sockaddr *addr, socklen_t addrlen);
struct MyTCP_Socket* my_accept(struct MyTCP_Socket * server_sock);
int my_send(struct MyTCP_Socket *sock, const void *buf, size_t len, int flags);
int my_recv(struct MyTCP_Socket *sock, void *buf, size_t len, int flags);
int my_close(struct MyTCP_Socket *sock);

#endif /* MYSOCKET_H */
