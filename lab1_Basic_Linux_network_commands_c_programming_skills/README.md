# Lab 1: Basic Linux Network Commands & C Programming Skills

This lab covers fundamental Linux networking commands and C programming exercises for file operations.

## Lab Structure

### 1. C_Program_File_Transfer

Contains C programs that demonstrate file copying using different approaches:
- **Functions approach**: Uses standard I/O functions (`fread`/`fwrite`)
- **System calls approach**: Uses low-level system calls (`read`/`write`)

Both programs:
- Copy text and binary files
- Measure execution time using `clock()` function
- Include error handling and proper file management

**See [C_Program_File_Transfer/README.md](C_Program_File_Transfer/README.md) for detailed usage instructions.**

### 2. Linux_Networking_Commands_OP

Contains output screenshots demonstrating various Linux networking commands:

- `ifconfig_output.png` - Network interface configuration
- `ifconfig-a_output.png` - All network interfaces
- `ifconfig_specific_interface.png` - Specific interface details
- `hostname_output.png` - System hostname
- `ping_output.png` - ICMP ping test
- `traceroute_output.png` - Network path tracing
- `netstat-a_output.png` - All network connections
- `netstat-r_output.png` - Routing table
- `route_output.png` - Route information
- `arp_output.png` - ARP table
- `dig_and_host_output.png` - DNS lookup commands
- `telnet_output.png` - Telnet connection test

### 3. Linux_Step_2_ping_op

Contains ping test results from different geographical regions:
- `Asia_ping_host.png` - Ping results to Asian hosts
- `Europe_ping_host.png` - Ping results to European hosts
- `NorthAmerica_ping_host.png` - Ping results to North American hosts

## Quick Start

### Compile and Run File Copy Programs

```bash
cd C_Program_File_Transfer
make                    # Compile both programs
make test              # Run tests with sample files
```

### View Networking Command Outputs

Navigate to `Linux_Networking_Commands_OP/` or `Linux_Step_2_ping_op/` directories to view the PNG screenshots of command outputs.

## Learning Objectives

- Understand Linux networking commands and their usage
- Implement file operations in C using both high-level functions and system calls
- Measure program execution time
- Handle binary and text files correctly
- Compare performance between different I/O approaches

## Requirements

- Linux/Unix environment
- GCC compiler
- Make utility (optional)
