# File Copy Programs

This directory contains two C programs that copy files (both text and binary) using different approaches and measure the execution time.

## Programs

1. **copy_file_functions** - Uses standard I/O functions (`fread`/`fwrite`)
2. **copy_file_system_calls** - Uses system calls (`read`/`write`)

Both programs:
- Copy files byte-by-byte (handles text and binary files)
- Measure execution time using `clock()` function
- Display source file, destination file, and time taken

## Compilation

### Using Makefile (Recommended)

Compile both programs:
```bash
make
```

Compile only the functions version:
```bash
make functions
```

Compile only the system calls version:
```bash
make syscalls
```

### Manual Compilation

**Functions version:**
```bash
gcc -Wall -Wextra -o Copy_files_functions/copy_file_functions Copy_files_functions/copy_file_functions.c
```

**System calls version:**
```bash
gcc -Wall -Wextra -o Copy_files_system_calls/copy_file_system_calls Copy_files_system_calls/copy_file_system_calls.c
```

## Running the Programs

### Functions Version
```bash
./Copy_files_functions/copy_file_functions <source_file> <destination_file>
```

**Example:**
```bash
./Copy_files_functions/copy_file_functions test_text.txt output.txt
```

### System Calls Version
```bash
./Copy_files_system_calls/copy_file_system_calls <source_file> <destination_file>
```

**Example:**
```bash
./Copy_files_system_calls/copy_file_system_calls test_binary.bin output.bin
```

## Testing

Run automated tests with provided test files:
```bash
make test
```

This will:
- Compile both programs
- Test with `test_text.txt` (text file)
- Test with `test_binary.bin` (binary file)
- Display timing results for each operation

## Makefile Targets

- `make` or `make all` - Compile both programs
- `make functions` - Compile only the functions version
- `make syscalls` - Compile only the system calls version
- `make test` - Compile and run tests with sample files
- `make clean` - Remove compiled executables and test output files

## Example Output

```
File copied successfully using functions (fread/fwrite)
Source: test_text.txt
Destination: output.txt
Time taken: 0.000007 seconds
```

## File Structure

```
C_Program_File_Transfer/
├── Copy_files_functions/
│   └── copy_file_functions.c
├── Copy_files_system_calls/
│   └── copy_file_system_calls.c
├── Makefile
├── README.md
├── test_text.txt          (sample text file)
└── test_binary.bin        (sample binary file)
```

## Requirements

- GCC compiler
- Linux/Unix environment (for system calls version)
- Make utility (optional, for using Makefile)
