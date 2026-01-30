/*
 * Concurrent TCP Server - Accepts multiple clients, spawns a thread per client
 * for file transfer. Each client sends a filename; server sends the file.
 * Usage: ./tcp_server <port>
 * Example: ./tcp_server 5000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define N 100
#define FILENAME_SIZE 256
#define FILE_BUFFER_SIZE 1024

int threadCount = 0;
pthread_t clients[N];

int sockfd;
int connfd;
struct sockaddr_in servAddr, clienAddr;
socklen_t clienLen = sizeof(clienAddr);

/* Structure passed to each thread (so connfd and client address are not overwritten) */
typedef struct {
    int connfd;
    struct sockaddr_in clientAddr;
} client_info_t;

void *connectionHandler(void *arg) {
    char filename[FILENAME_SIZE];
    char buffer[FILE_BUFFER_SIZE];
    FILE *file;
    size_t bytes_read;
    uint32_t file_size, file_size_net;
    long file_len;

    client_info_t *info = (client_info_t *)arg;
    int conn = info->connfd;
    struct sockaddr_in clientAddr = info->clientAddr;

    /* Connection established */
    printf("Connection established with client IP: %s and Port: %d\n",
           inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    /* Receive filename from client */
    memset(filename, 0, FILENAME_SIZE);
    if (recv(conn, filename, FILENAME_SIZE, 0) <= 0) {
        perror("Receive filename failed");
        close(conn);
        free(info);
        pthread_exit(0);
    }

    /* Open file and send to client */
    file = fopen(filename, "rb");
    if (file == NULL) {
        file_size_net = htonl(0);
        send(conn, &file_size_net, sizeof(file_size_net), 0);
        printf("File not found: %s\n", filename);
        close(conn);
        free(info);
        pthread_exit(0);
    }

    /* Get file size */
    fseek(file, 0, SEEK_END);
    file_len = ftell(file);
    fseek(file, 0, SEEK_SET);
    file_size = (uint32_t)file_len;
    file_size_net = htonl(file_size);
    send(conn, &file_size_net, sizeof(file_size_net), 0);

    /* Read file and send to connection descriptor */
    while ((bytes_read = fread(buffer, 1, FILE_BUFFER_SIZE, file)) > 0) {
        if (send(conn, buffer, bytes_read, 0) < 0) {
            perror("Send file failed");
            break;
        }
    }

    printf("File transfer complete: %s\n", filename);
    fclose(file);
    close(conn);
    free(info);
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    int port;
    pthread_attr_t attr;

    if (argc != 2) {
        printf("Usage: %s <port #>\n", argv[0]);
        exit(0);
    }
    port = atoi(argv[1]);

    /* Open a TCP socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    /* Setup server address to bind */
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(port);

    /* Bind IP address and port for server endpoint socket */
    if (bind(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(1);
    }

    /* Server listening; queue up to 5 client requests */
    if (listen(sockfd, 5) < 0) {
        perror("Listen failed");
        close(sockfd);
        exit(1);
    }
    printf("Server listening/waiting for client at port %d\n", port);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    while (1) {
        connfd = accept(sockfd, (struct sockaddr *)&clienAddr, &clienLen);
        if (connfd < 0) {
            perror("Accept failed");
            continue;
        }

        /* Pass copy of connfd and client address to thread */
        client_info_t *info = (client_info_t *)malloc(sizeof(client_info_t));
        if (info == NULL) {
            perror("malloc failed");
            close(connfd);
            continue;
        }
        info->connfd = connfd;
        info->clientAddr = clienAddr;

        if (pthread_create(&clients[threadCount % N], &attr, connectionHandler, (void *)info) < 0) {
            perror("Unable to create thread");
            free(info);
            close(connfd);
            exit(1);
        }
        printf("Thread %d has been created to service client request\n", ++threadCount);
    }
}
