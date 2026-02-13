// UDP Server with stop-and-wait rdt3.0 protocol
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

#define PLOSTMSG 5

typedef struct {
    int seq_ack;
    int len;
    int cksum;
} Header;

typedef struct {
    Header header;
    char data[10];
} Packet;

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

void printPacket(Packet packet) {
    printf("Packet{ header: { seq_ack: %d, len: %d, cksum: %d }, data: \"",
           packet.header.seq_ack,
           packet.header.len,
           packet.header.cksum);
    fwrite(packet.data, (size_t)packet.header.len, 1, stdout);
    printf("\" }\n");
}

// Send ACK to client
void serverSend(int sockfd, const struct sockaddr *address, socklen_t addrlen,
                int seqnum) {
    if (rand() % PLOSTMSG == 0) {
        printf("Dropping ACK\n");
    } else {
        Packet packet;
        memset(&packet, 0, sizeof(packet));
        packet.header.seq_ack = seqnum;
        packet.header.len = 0;

        // Simulate corrupted checksum sometimes
        if (rand() % 5 == 0) {
            packet.header.cksum = 0;
            printf("Server: Simulating corrupted ACK checksum\n");
        } else {
            packet.header.cksum = getChecksum(packet);
        }

        sendto(sockfd, &packet, sizeof(packet), 0, address, addrlen);
        printf("Sent ACK %d, checksum %d\n", packet.header.seq_ack,
               packet.header.cksum);
    }
}

// Receive packet from client, validate, and return when good packet received
Packet serverReceive(int sockfd, struct sockaddr *address, socklen_t *addrlen,
                     int seqnum, int fp) {
    Packet packet;
    int last_ack_sent = (seqnum + 1) % 2;  // Previous seq for resending ACK

    while (1) {
        memset(&packet, 0, sizeof(packet));
        ssize_t n = recvfrom(sockfd, &packet, sizeof(packet), 0, address,
                             addrlen);

        if (n < (ssize_t)sizeof(Header)) {
            printf("Received invalid packet (too short)\n");
            serverSend(sockfd, address, *addrlen, last_ack_sent);
            continue;
        }

        printf("Received: ");
        printPacket(packet);

        int expected_cksum = getChecksum(packet);

        if (packet.header.cksum != expected_cksum) {
            printf("Bad checksum, expected %d\n", expected_cksum);
            serverSend(sockfd, address, *addrlen, last_ack_sent);
        } else if (packet.header.seq_ack != seqnum) {
            printf("Bad seqnum, expected %d\n", seqnum);
            serverSend(sockfd, address, *addrlen, last_ack_sent);
        } else {
            printf("Good packet\n");
            last_ack_sent = seqnum;
            serverSend(sockfd, address, *addrlen, seqnum);

            if (packet.header.len > 0) {
                write(fp, packet.data, packet.header.len);
            }

            return packet;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <outfile>\n", argv[0]);
        exit(1);
    }

    srand((unsigned)time(NULL));

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        exit(1);
    }

    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(atoi(argv[1]));

    if (bind(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(1);
    }

    printf("Server listening on port %s\n", argv[1]);

    int fp = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fp < 0) {
        perror("File failed to open");
        close(sockfd);
        exit(1);
    }

    int seqnum = 0;
    Packet packet;
    struct sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);

    do {
        packet = serverReceive(sockfd, (struct sockaddr *)&clientAddr,
                               &addrlen, seqnum, fp);
        seqnum = (seqnum + 1) % 2;
    } while (packet.header.len > 0);

    printf("File transfer complete\n");

    close(fp);
    close(sockfd);
    return 0;
}
