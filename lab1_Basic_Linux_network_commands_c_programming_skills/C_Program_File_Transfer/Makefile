# Makefile for File Copy Programs

CC = gcc
CFLAGS = -Wall -Wextra -std=c11

# Directories
FUNCTIONS_DIR = Copy_files_functions
SYSCALLS_DIR = Copy_files_system_calls

# Targets
all: functions syscalls

functions:
	$(CC) $(CFLAGS) -o $(FUNCTIONS_DIR)/copy_file_functions $(FUNCTIONS_DIR)/copy_file_functions.c

syscalls:
	$(CC) $(CFLAGS) -o $(SYSCALLS_DIR)/copy_file_system_calls $(SYSCALLS_DIR)/copy_file_system_calls.c

clean:
	rm -f $(FUNCTIONS_DIR)/copy_file_functions
	rm -f $(SYSCALLS_DIR)/copy_file_system_calls
	rm -f *_copy.txt *_copy.bin

test: all
	@echo "=== Testing Functions Approach ==="
	./$(FUNCTIONS_DIR)/copy_file_functions test_text.txt test_text_functions_copy.txt
	./$(FUNCTIONS_DIR)/copy_file_functions test_binary.bin test_binary_functions_copy.bin
	@echo ""
	@echo "=== Testing System Calls Approach ==="
	./$(SYSCALLS_DIR)/copy_file_system_calls test_text.txt test_text_syscalls_copy.txt
	./$(SYSCALLS_DIR)/copy_file_system_calls test_binary.bin test_binary_syscalls_copy.bin

.PHONY: all functions syscalls clean test
