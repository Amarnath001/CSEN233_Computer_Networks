# Lab 2: TCP Client–Server Implementation

## Overview

- **tcp_client**: Connects to the server and requests a file to download. The server sends the file over the TCP connection.
- **tcp_server**: Concurrent TCP server that accepts multiple clients. For each client it spawns a thread to handle the file transfer on that connection, then continues listening for more clients.

## Compile

```bash
make
```

## Run

### Server (run first)

Server listens on the given port. Place files you want to serve in the same directory as the server.

```bash
./tcp_server <port>
# Example:
./tcp_server 5000
```

### Client

The client saves the downloaded file in a **`downloads/`** folder under the current working directory (created automatically if it doesn’t exist). So when you run the client from the lab2 folder, the file is written as `downloads/sample_file.txt`. That keeps downloaded files separate so you can easily compare them to the server’s original (e.g. `diff sample_file.txt downloads/sample_file.txt`). Running `make clean` removes the `downloads/` folder and its contents.

Use `127.0.0.1` when client and server run on the same machine. Use a classmate’s IP when they run the server on the same network.

```bash
./tcp_client <server_ip> <port> <filename>
# Same machine:
./tcp_client 127.0.0.1 5000 sample_file.txt
# Classmate’s server (same network):
./tcp_client 192.168.1.10 5000 sample_file.txt
```

## Verifying the download (diff)

The client saves files under **`downloads/`**, so the original (e.g. `sample_file.txt` in the server directory) is never overwritten. To check that the download is identical:

```bash
# From the lab2 directory (server file and downloads/ are both here):
diff sample_file.txt downloads/sample_file.txt
```

- If there is **no output**, the two files are identical.
- If there are differences, `diff` will print them.

For binary files you can use:

```bash
cmp sample_file.txt downloads/sample_file.txt
```

No output means the files are identical; otherwise `cmp` reports the first byte where they differ.

## Protocol (brief)

1. Client connects and sends the requested **filename** (fixed 256-byte buffer).
2. Server tries to open the file:
   - If it fails: sends file size `0` (4 bytes), then closes the connection.
   - If it succeeds: sends file size (4 bytes, network order), then the file contents.
3. Client receives the 4-byte size; if non-zero, it receives that many bytes and writes them to a local file with the same name.

## Notes

- Server uses a thread per client (up to N clients). Threads are created detached.
- For local testing, run the server in one terminal and the client in another, using `127.0.0.1` and the same port.
- To test with a classmate, run the server on one machine and the client on another, using the server machine’s IP and the same port.
