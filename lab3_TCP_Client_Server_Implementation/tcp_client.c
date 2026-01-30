/*
 * TCP Client - Connects to server and requests a file download.
 * Usage: ./tcp_client <server_ip> <port> <filename>
 * Example: ./tcp_client 127.0.0.1 5000 myfile.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define FILENAME_SIZE 256
#define FILE_BUFFER_SIZE 1024
#define DOWNLOAD_DIR "downloads"

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in serverAddr;
    char filename[FILENAME_SIZE];
    char buffer[FILE_BUFFER_SIZE];
    ssize_t bytes_received;
    FILE *outfile;
    uint32_t file_size, remaining;
    uint32_t size_net;

    if (argc != 4) {
        printf("Usage: %s <server_ip> <port> <filename>\n", argv[0]);
        exit(1);
    }

    /* Create TCP socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    /* Setup server address */
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &serverAddr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sockfd);
        exit(1);
    }

    /* Connect to server */
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        exit(1);
    }
    printf("Connected to server %s:%s\n", argv[1], argv[2]);

    /* Send requested filename to server */
    memset(filename, 0, FILENAME_SIZE);
    strncpy(filename, argv[3], FILENAME_SIZE - 1);
    if (send(sockfd, filename, FILENAME_SIZE, 0) < 0) {
        perror("Send filename failed");
        close(sockfd);
        exit(1);
    }
    printf("Requested file: %s\n", filename);

    /* Receive file size (4 bytes, network byte order) */
    if (recv(sockfd, &size_net, sizeof(size_net), 0) != sizeof(size_net)) {
        perror("Receive file size failed");
        close(sockfd);
        exit(1);
    }
    file_size = ntohl(size_net);

    if (file_size == 0) {
        printf("Server reported: file not found.\n");
        close(sockfd);
        exit(1);
    }

    /* Create downloads directory if it doesn't exist */
    if (mkdir(DOWNLOAD_DIR, 0755) < 0 && errno != EEXIST) {
        perror("Cannot create download directory");
        close(sockfd);
        exit(1);
    }

    /* Build path: downloads/<filename> and open for writing */
    {
        char filepath[FILENAME_SIZE + sizeof(DOWNLOAD_DIR) + 2];
        snprintf(filepath, sizeof(filepath), "%s/%s", DOWNLOAD_DIR, filename);
        outfile = fopen(filepath, "wb");
    }
    if (outfile == NULL) {
        perror("Cannot create local file");
        close(sockfd);
        exit(1);
    }

    /* Receive file content */
    remaining = file_size;
    while (remaining > 0) {
        size_t to_read = remaining < FILE_BUFFER_SIZE ? remaining : FILE_BUFFER_SIZE;
        bytes_received = recv(sockfd, buffer, to_read, 0);
        if (bytes_received <= 0) {
            perror("Receive file content failed");
            fclose(outfile);
            close(sockfd);
            exit(1);
        }
        fwrite(buffer, 1, bytes_received, outfile);
        remaining -= bytes_received;
    }

    fclose(outfile);
    close(sockfd);
    printf("File '%s' downloaded to %s/ (%u bytes).\n", filename, DOWNLOAD_DIR, file_size);
    return 0;
}
