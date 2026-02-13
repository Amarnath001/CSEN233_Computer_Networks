// UDP client with stop-and-wait rdt3.0 protocol
// Packets have checksum, sequence number, acknowledgement number, and timer
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/types.h>

// Header: sequence/acknowledgement number, checksum, and length of packet
typedef struct {
    int seq_ack;
    int len;
    int cksum;
} Header;

// Packet: header + data
typedef struct {
    Header header;
    char data[10];
} Packet;

// Calculate checksum (longitudinal parity - XOR of all bytes)
// Checksum field must be 0 when computing
int getChecksum(Packet packet) {
    packet.header.cksum = 0;
    int checksum = 0;
    char *ptr = (char *)&packet;
    char *end = ptr + sizeof(Header) + packet.header.len;
    while (ptr < end) {
        checksum ^= *ptr++;
    }
    return checksum;
}

// Print received packet
void printPacket(Packet packet) {
    printf("Packet{ header: { seq_ack: %d, len: %d, cksum: %d }, data: \"",
            packet.header.seq_ack,
            packet.header.len,
            packet.header.cksum);
    fwrite(packet.data, (size_t)packet.header.len, 1, stdout);
    printf("\" }\n");
}

// Client sends packet with checksum and sequence number,
// waits for acknowledgement with select() timer, retransmits on timeout or bad ACK
void clientSend(int sockfd, const struct sockaddr *address, socklen_t addrlen,
                Packet packet, unsigned retries) {
    (void)retries;  /* unused, retry until ACK received */
    while (1) {
        // Calculate and set checksum
        packet.header.cksum = getChecksum(packet);

        // Simulate loss: sometimes send wrong checksum (bit error)
        if (rand() % 5 == 0) {
            packet.header.cksum = 0;  // Corrupt checksum
            printf("Client: Simulating corrupted checksum\n");
        }

        // Simulate packet loss (probability = 20%)
        if (rand() % 5 == 0) {
            printf("Dropping packet\n");
        } else {
            printf("Client sending packet (seq=%d, len=%d)\n",
                   packet.header.seq_ack, packet.header.len);
            sendto(sockfd, &packet, sizeof(packet), 0, address, addrlen);
        }

        // Wait for ACK using select()
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        int rv = select(sockfd + 1, &readfds, NULL, NULL, &tv);

        if (rv == 0) {
            printf("Timeout\n");
            retries++;
        } else if (rv > 0) {
            // Receive ACK from server
            Packet recvpacket;
            socklen_t recv_addrlen = addrlen;
            ssize_t n = recvfrom(sockfd, &recvpacket, sizeof(recvpacket), 0,
                                 (struct sockaddr *)address, &recv_addrlen);

            if (n < 0) {
                printf("Client: recvfrom error\n");
                retries++;
                continue;
            }

            printf("Client received ACK %d, checksum %d\n",
                   recvpacket.header.seq_ack, recvpacket.header.cksum);

            // Verify checksum of received ACK
            int expected_cksum = getChecksum(recvpacket);

            if (recvpacket.header.cksum != expected_cksum) {
                printf("Client: Bad checksum, expected checksum was: %d\n",
                       expected_cksum);
                retries++;
            } else if (recvpacket.header.seq_ack != packet.header.seq_ack) {
                printf("Client: Bad seqnum, expected sequence number was: %d\n",
                       packet.header.seq_ack);
                retries++;
            } else {
                printf("Client: Good ACK\n");
                break;
            }
        } else {
            printf("Client: select error\n");
            retries++;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <ip> <port> <srcfile>\n", argv[0]);
        exit(0);
    }

    srand((unsigned)time(NULL));

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        exit(1);
    }

    struct sockaddr_in servAddr;
    struct hostent *host;
    host = gethostbyname(argv[1]);
    if (host == NULL) {
        perror("Failed to resolve hostname");
        close(sockfd);
        exit(1);
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(atoi(argv[2]));
    memcpy(&servAddr.sin_addr, host->h_addr_list[0], host->h_length);

    int fp = open(argv[3], O_RDONLY);
    if (fp < 0) {
        perror("Failed to open file");
        close(sockfd);
        exit(1);
    }

    // Send file contents packet by packet
    int seq = 0;
    socklen_t addr_len = sizeof(servAddr);
    Packet packet;

    int bytes;
    while ((bytes = read(fp, packet.data, sizeof(packet.data))) > 0) {
        packet.header.seq_ack = seq;
        packet.header.len = bytes;
        clientSend(sockfd, (struct sockaddr *)&servAddr, addr_len, packet, 0);
        seq = (seq + 1) % 2;
    }

    // Send zero-length packet to signal file complete
    Packet final;
    memset(&final, 0, sizeof(final));
    final.header.seq_ack = seq;
    final.header.len = 0;
    final.header.cksum = getChecksum(final);
    clientSend(sockfd, (struct sockaddr *)&servAddr, addr_len, final, 0);

    close(fp);
    close(sockfd);
    return 0;
}
