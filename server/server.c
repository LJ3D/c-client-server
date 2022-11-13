#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <syscall.h>

#define PORT 52727
#define BUFFER_SIZE 1024


/*
handle_client is created in a new thread when a client connects to the server.
currently, it just echos back whatever the client sends.
*/

void *handle_client(void* p_fd){
    int thread_id = syscall(SYS_gettid);
    int fd = *(int*)p_fd; /* dereference the pointer to get the actual file descriptor */
    char buffer[BUFFER_SIZE]; /* holds data read from the client */
    free(p_fd); /* free the memory allocated in main() */
    /* main loop, reads data from socket then sends it back */
    while(1){
        int bytes_read;
        int bytes_sent;
        bytes_read = recv(fd, &buffer, BUFFER_SIZE, 0);
        if(bytes_read == 0){
            printf("(thread %d): RECV: Client disconnected (probably)\n", thread_id);
            break;
        }
        printf("Thread %d received %d bytes from client %d\n", thread_id, bytes_sent, fd);
        bytes_sent = send(fd, &buffer, BUFFER_SIZE, 0);
        if(bytes_sent == 0){
            printf("(thread %d): SEND: Client disconnected (probably)\n", thread_id);
            break;
        }
        printf("Thread %d sent %d bytes to client %d\n", thread_id, bytes_sent, fd);
    }
    close(fd);
    return NULL;
}

int main(int argc, char const* argv[]){
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    signal(SIGPIPE, SIG_IGN); /* Ignore SIGPIPE, this prevents the server from crashing when a client disconnects */
    pthread_sigmask(SIGPIPE, NULL, NULL);

    printf("Opening server....\n");
    /* create a socket to listen for incoming connections on */
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("ERR: socket() failed");
        exit(1);
    }
    /* set up the socket options */
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))){
        perror("ERR: setsockopt() failed");
        exit(1);
    }
    /* set up the address */
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    /* bind the socket to the port */
    if(bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0){
        perror("ERR: bind() failed");
        exit(1);
    }
    /* listen for incoming connections */
    if(listen(server_fd, 16) < 0){
        perror("ERR: listen() failed");
        exit(1);
    }
    /* start receiving connections and creating threads for them */
    printf("Waiting for connections...\n");
    while(1){
        int new_socket_fd;
        pthread_t new_thread;
        int* client_fd = malloc(sizeof(int)); /* This memory is freed when handle_client is called */
        int addrlen = sizeof(address);
        if((new_socket_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0){
            perror("ERR: accept() failed");
            exit(1);
        }
        *client_fd = new_socket_fd; /* Get a pointer to the new socket file descriptor */
        printf("New connection accepted, creating new thread to handle it \n");
        pthread_create(&new_thread, NULL, handle_client, client_fd);
    }
    close(server_fd);
    return 0;
}