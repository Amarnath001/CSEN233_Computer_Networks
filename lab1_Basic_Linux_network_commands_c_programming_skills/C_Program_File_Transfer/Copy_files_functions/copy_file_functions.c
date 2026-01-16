/*
 * File Copy Program using Functions (fread/fwrite)
 * This program copies files (both text and binary) using standard I/O functions
 * and measures the time taken for the copy operation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define BUFFER_SIZE 4096  // Buffer size for reading/writing

int main(int argc, char *argv[]) {
    FILE *source_file, *dest_file;
    char buffer[BUFFER_SIZE];
    size_t bytes_read, bytes_written;
    clock_t start, end;
    double cpu_time_used;

    // Check for correct number of arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_file> <destination_file>\n", argv[0]);
        exit(1);
    }

    // Open source file for reading
    source_file = fopen(argv[1], "rb");  // "rb" mode for binary files (works for text too)
    if (source_file == NULL) {
        perror("Error opening source file");
        exit(1);
    }

    // Open destination file for writing
    dest_file = fopen(argv[2], "wb");  // "wb" mode for binary files (works for text too)
    if (dest_file == NULL) {
        perror("Error opening destination file");
        fclose(source_file);
        exit(1);
    }

    // Start timing
    start = clock();

    // Copy file content using fread and fwrite
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, source_file)) > 0) {
        bytes_written = fwrite(buffer, 1, bytes_read, dest_file);
        if (bytes_written != bytes_read) {
            fprintf(stderr, "Error writing to destination file\n");
            fclose(source_file);
            fclose(dest_file);
            exit(1);
        }
    }

    // Check for read errors
    if (ferror(source_file)) {
        perror("Error reading from source file");
        fclose(source_file);
        fclose(dest_file);
        exit(1);
    }

    // End timing
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    // Close files
    fclose(source_file);
    fclose(dest_file);

    // Display results
    printf("File copied successfully using functions (fread/fwrite)\n");
    printf("Source: %s\n", argv[1]);
    printf("Destination: %s\n", argv[2]);
    printf("Time taken: %.6f seconds\n", cpu_time_used);

    return 0;
}
