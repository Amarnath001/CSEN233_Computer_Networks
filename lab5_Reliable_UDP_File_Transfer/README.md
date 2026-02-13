# Lab 5: Stop-and-Wait RDT3.0 over UDP

Reliable data transfer implementation using the stop-and-wait rdt3.0 protocol over UDP. Handles packet loss and bit errors through checksums, sequence numbers, timeouts, and retransmissions.

## Protocol

- **Header**: seq_ack (32 bits), len (32 bits), checksum (32 bits)
- **Packet**: header + data (up to 10 bytes)
- **Sequence numbers**: 0 and 1 (alternating)
- **Checksum**: Longitudinal parity (XOR of all bytes)
- **Timer**: select() with 1-second timeout for retransmission

## Build

```bash
make
```

## Usage

**Terminal 1** - Start the server first:
```bash
./udp_server <port> <outfile>
```

**Terminal 2** - Run the client:
```bash
./udp_client <ip> <port> <srcfile>
```

## Example

```bash
# Terminal 1
./udp_server 5000 received_file.txt

# Terminal 2
./udp_client localhost 5000 sample_file.txt
```

## Verification

The implementation includes random simulation of:
- Packet loss (20% on client)
- ACK loss (20% on server)
- Corrupted checksums (20% on both sides)

Successful transfers demonstrate that the protocol recovers from these errors via timeouts and retransmissions.
