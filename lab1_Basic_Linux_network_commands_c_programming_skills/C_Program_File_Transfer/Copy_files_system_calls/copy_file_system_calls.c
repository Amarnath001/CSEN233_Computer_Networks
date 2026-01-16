/*
 * File Copy Program using System Calls (read/write)
 * This program copies files (both text and binary) using system calls
 * and measures the time taken for the copy operation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 4096  // Buffer size for reading/writing

int main(int argc, char *argv[]) {
    int source_fd, dest_fd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_written;
    clock_t start, end;
    double cpu_time_used;

    // Check for correct number of arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_file> <destination_file>\n", argv[0]);
        exit(1);
    }

    // Open source file for reading
    source_fd = open(argv[1], O_RDONLY);
    if (source_fd == -1) {
        perror("Error opening source file");
        exit(1);
    }

    // Open destination file for writing (create if doesn't exist, truncate if exists)
    dest_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd == -1) {
        perror("Error opening destination file");
        close(source_fd);
        exit(1);
    }

    // Start timing
    start = clock();

    // Copy file content using read and write system calls
    while ((bytes_read = read(source_fd, buffer, BUFFER_SIZE)) > 0) {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written == -1) {
            perror("Error writing to destination file");
            close(source_fd);
            close(dest_fd);
            exit(1);
        }
        if (bytes_written != bytes_read) {
            fprintf(stderr, "Warning: Partial write occurred\n");
        }
    }

    // Check for read errors
    if (bytes_read == -1) {
        perror("Error reading from source file");
        close(source_fd);
        close(dest_fd);
        exit(1);
    }

    // End timing
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    // Close file descriptors
    close(source_fd);
    close(dest_fd);

    // Display results
    printf("File copied successfully using system calls (read/write)\n");
    printf("Source: %s\n", argv[1]);
    printf("Destination: %s\n", argv[2]);
    printf("Time taken: %.6f seconds\n", cpu_time_used);

    return 0;
}
