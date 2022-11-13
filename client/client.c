#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <syscall.h>

#define BUFFER_SIZE 1024

int main(int argc, char const* argv[]){
    int client_fd;
    struct sockaddr_in server_address;
    int opt = 1;
    char ip_address[17] = {0};
    char port_arr[6] = {0};
    int port = 0;
    signal(SIGPIPE, SIG_IGN); /* Ignore SIGPIPE, this prevents the program from crashing if the server disconnects */
    /* get connection info from user */
    printf("Enter IP address of server to connect to: ");
    scanf("%16s", ip_address);
    printf("Enter the port to connect through: ");
    scanf("%5s", port_arr);
    sscanf(port_arr, "%d", &port);
    /* set up server address sockaddr_in struct */
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    if(inet_pton(AF_INET, ip_address, &server_address.sin_addr) <= 0){
        perror("ERR: invalid address (inet_pton() failed)");
        exit(1);
    }
    /* create a socket to connect to the server with */
    if((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("ERR: socket() failed");
        exit(1);
    }
    /* connect to the server */
    if(connect(client_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0){
        perror("ERR: connect() failed");
        exit(1);
    }
    /* start up main loop and do some stuff! */
    while(1){
        char buffer[BUFFER_SIZE] = {0}; /* Used to store incoming data */
        int bytes_sent;
        int bytes_received;
        /* send something */
        bytes_sent = send(client_fd, "Hello!", 6, 0);
        if(bytes_sent <= 0){
            printf("Failed to send data to the server, exiting...\n");
            break;
        }else{
            printf("Sent %d bytes to the server\n", bytes_sent);
        }
        /* receive something back */
        bytes_received = recv(client_fd, &buffer, BUFFER_SIZE, 0);
        if(bytes_received <= 0){
            printf("Failed to receive data from the server, exiting...\n");
            break;
        }else{
            printf("Received %d bytes from the server: %s\n", bytes_received, buffer);
        }
        sleep(3); /* Dont spam! */
    }
    /* close the client file descriptor */
    close(client_fd);
    return 0;
}