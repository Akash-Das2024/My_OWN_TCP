#include "mysocket.h"

// Function to handle sending struct Messages
void *send_thread(void *arg)
{
    struct MyTCP_Socket *sock = (struct MyTCP_Socket *)arg;
    
    
    pthread_mutex_lock(&sock->lock);
    while (!sock->closed)
    {
        usleep(1000); // Sleep for some time T
        while (sock->send_buffer.front != sock->send_buffer.rear)
        {
            struct Message msg = sock->send_buffer.messages[sock->send_buffer.front];
            int sent = 0;
            while (sent < msg.length)
            {
                int n = send(sock->sockfd, msg.data + sent, msg.length - sent, 0);
                if (n <= 0)
                {
                    // Handle send errors here
                    break;
                }
                sent += n;
            }
            sock->send_buffer.front = (sock->send_buffer.front + 1) % MAX_SEND_BUFFER_SIZE;
        }
    }
    pthread_mutex_unlock(&sock->lock);
    return NULL;
}

// Function to handle receiving struct Messages
void *receive_thread(void *arg)
{
    struct MyTCP_Socket *sock = (struct MyTCP_Socket *)arg;
    
    
    pthread_mutex_lock(&sock->lock);
    while (!sock->closed)
    {
        char buf[MAX_MESSAGE_LENGTH];
        int n = recv(sock->sockfd, buf, sizeof(buf), 0);
        if (n <= 0)
        {
            // Handle receive errors here
            break;
        }
        int bytes_received = n;
        int total_bytes_received = n;
        while (total_bytes_received < MAX_MESSAGE_LENGTH)
        {
            n = recv(sock->sockfd, buf + bytes_received, sizeof(buf) - bytes_received, MSG_DONTWAIT);
            if (n <= 0)
            {
                break;
            }
            total_bytes_received += n;
            bytes_received += n;
        }
        struct Message msg;
        memcpy(msg.data, buf, total_bytes_received);
        msg.length = total_bytes_received;
        sock->received_buffer.messages[sock->received_buffer.rear] = msg;
        sock->received_buffer.rear = (sock->received_buffer.rear + 1) % MAX_SEND_BUFFER_SIZE;
    }
    pthread_mutex_unlock(&sock->lock);
    return NULL;
}

// Function to create a MyTCP socket
struct MyTCP_Socket * my_socket(int domain, int type, int protocol)
{
    if (type != SOCK_MyTCP)
    {
        // Handle non-MyTCP socket types
        return NULL;
    }
    int sockfd = socket(domain, SOCK_STREAM, protocol);
    if (sockfd < 0)
    {
        // Handle socket creation errors here
        return NULL;
    }
   struct MyTCP_Socket* sock = (struct MyTCP_Socket *)malloc(sizeof(struct MyTCP_Socket));
   if (sock == NULL) {
    perror("Error allocating memory for socket");
    exit(1);
}

   int err1 = pthread_mutex_init(&sock->lock, NULL);
if (err1 != 0)
{
    // Handle mutex initialization errors here
    return NULL;
}

   
   
    sock->sockfd = sockfd;
    sock->send_buffer.front = 0;
    sock->send_buffer.rear = 0;
    sock->received_buffer.front = 0;
    sock->received_buffer.rear = 0;
    sock->closed = false;
    int err = pthread_create(&sock->send_thread, NULL, send_thread, (void *)sock);
    if (err != 0)
    {
        // Handle thread creation errors here
        return NULL;
    }
    err = pthread_create(&sock->receive_thread, NULL, receive_thread, (void *)sock);
    if (err != 0)
    {
        // Handle thread creation errors here
        return NULL;
    }
    return sock;
}


/*struct MyTCP_Socket * my_socket(int domain, int type, int protocol)
{
    // Check the socket type
    if (type != SOCK_MyTCP)
    {
        // Handle non-MyTCP socket types
        return -1;
    }

    // Check the socket domain
    if (domain != AF_INET && domain != AF_INET6)
    {
        // Handle invalid socket domains
        return -1;
    }

    // Check the protocol
    if (protocol != IPPROTO_TCP)
    {
        // Handle invalid protocols
        return -1;
    }

    // Create a new socket
    int sockfd = socket(domain, SOCK_STREAM, protocol);
    if (sockfd < 0)
    {
        // Handle socket creation errors here
        return sockfd;
    }

    // Allocate memory for the MyTCP_Socket struct
    struct MyTCP_Socket *sock = (struct MyTCP_Socket *)malloc(sizeof(struct MyTCP_Socket));
    if (sock == NULL)
    {
        // Handle memory allocation errors here
        return -1;
    }

    // Initialize the MyTCP_Socket struct
    sock->sockfd = sockfd;
    sock->send_buffer.front = 0;
    sock->send_buffer.rear = 0;
    sock->received_buffer.front = 0;
    sock->received_buffer.rear = 0;
    sock->closed = false;

    // Create the send thread
    int err = pthread_create(&sock->send_thread, NULL, send_thread, (void *)sock);
    if (err != 0)
    {
        // Handle thread creation errors here
        return -1;
    }

    // Create the receive thread
    err = pthread_create(&sock->receive_thread, NULL, receive_thread, (void *)sock);
    if (err != 0)
    {
        // Handle thread creation errors here
        return -1;
    }

    // Return a pointer to the MyTCP_Socket struct
    return sock;
}*/


int my_bind(struct MyTCP_Socket *sock, const char* address, int port) {
    // Create a sockaddr_in structure for the address-port combination
    //struct MyTCP_Socket *sock = (struct MyTCP_Socket *)sockfd;
    struct sockaddr_in serv_addr;
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(address);
    serv_addr.sin_port = htons(port);

    // Bind the socket with the address-port combination
    int bind_res = bind(sock->sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    if (bind_res < 0) {
        perror("ERROR on binding");
    }
    return bind_res;
}

int my_listen(struct MyTCP_Socket *sock, int backlog) {
    // Listen for connections

   // struct MyTCP_Socket *sock = (struct MyTCP_Socket *)sockfd;
    int listen_res = listen(sock->sockfd, backlog);
    if (listen_res < 0) {
        perror("ERROR on listening");
    }
    return listen_res;
}





// Function to connect to a MyTCP server
int my_connect(struct MyTCP_Socket * sock, const struct sockaddr *addr, socklen_t addrlen)
{
   
    int err = connect(sock->sockfd, addr, addrlen);
    if (err < 0)
    {
        // Handle connect errors here
        return err;
    }
    return 0;
}



struct MyTCP_Socket* my_accept(struct MyTCP_Socket * server_sock) {
    // Accept a connection on the TCP socket

    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    int newsockfd = accept(server_sock->sockfd, (struct sockaddr*) &cli_addr, &clilen);
    if (newsockfd < 0) {
        perror("ERROR on accept");
        return NULL;
    }

    // Create a new MyTCP socket for the accepted connection
    struct MyTCP_Socket *client_sock = (struct MyTCP_Socket *)malloc(sizeof(struct MyTCP_Socket));
   if (client_sock == NULL) {
    perror("Error allocating memory for socket");
    exit(1);
}


    client_sock->sockfd = newsockfd;
    client_sock->closed = false;
    client_sock->send_buffer.front = 0;
    client_sock->send_buffer.rear = 0;
    client_sock->received_buffer.front = 0;
    client_sock->received_buffer.rear = 0;

    return  client_sock;
}










// Function to send a struct Message through a MyTCP socket
int my_send(struct MyTCP_Socket *sock, const void *buf, size_t len, int flags)
{
    
    if (sock->closed)
    {
        // Handle closed socket errors here
        return -1;
    }
    if (len > MAX_MESSAGE_LENGTH)
    {
        // Handle struct Message too long errors here
        return -1;
    }
    if ((sock->send_buffer.rear + 1) % MAX_SEND_BUFFER_SIZE == sock->send_buffer.front)
    {
        // Handle send buffer full errors here
        return -1;
    }
    struct Message msg;
    memcpy(msg.data, buf, len);
    msg.length = len;
    sock->send_buffer.messages[sock->send_buffer.rear] = msg;
    sock->send_buffer.rear = (sock->send_buffer.rear + 1) % MAX_SEND_BUFFER_SIZE;
    return len;
}

// Function to receive a struct Message through a MyTCP socket
int my_recv(struct MyTCP_Socket *sock, void *buf, size_t len, int flags)
{
    //struct MyTCP_Socket *sock = (struct MyTCP_Socket *)sockfd;
    if (sock->closed)
    {
        // Handle closed socket errors here
        return -1;
    }
    if (sock->received_buffer.front == sock->received_buffer.rear)
    {
        // Handle empty receive buffer errors here
        return -1;
    }
    struct Message msg = sock->received_buffer.messages[sock->received_buffer.front];
    if (len < msg.length)
    {
        // Handle buffer too small errors here
        return -1;
    }
    memcpy(buf, msg.data, msg.length);
    sock->received_buffer.front = (sock->received_buffer.front + 1) % MAX_SEND_BUFFER_SIZE;
    return msg.length;
}

// Function to close a MyTCP socket
int my_close(struct MyTCP_Socket *sock)
{
    //struct MyTCP_Socket *sock = (struct MyTCP_Socket *)sockfd;
    sock->closed = true;
    close(sock->sockfd);
    pthread_mutex_destroy(&sock->lock);
    pthread_join(sock->send_thread, NULL);
    pthread_join(sock->receive_thread, NULL);
    free(sock);
    return 0;
}
