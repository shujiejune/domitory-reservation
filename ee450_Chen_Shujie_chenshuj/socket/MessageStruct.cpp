#include "MessageStruct.h"
int socket_send_payload(int sockfd, const void *payload, size_t dataLength)
{
    if (send(sockfd, &dataLength, sizeof(dataLength), 0) == -1)
    {
        perror("Failed to send payload size");
        return -1;
    }

    if (send(sockfd, payload, dataLength, 0) == -1)
    {
        perror("Failed to send payload data");
        return -1;
    }

    return 0;
}

int socket_receive_payload(int sockfd, void *packet, size_t *dataLength)
{
    // recv packet length
    size_t size;
    int n = recv(sockfd, &size, sizeof(size), 0);
    if (n < 0)
    {
        perror("Failed to receive payload size");
        return -1;
    }
    else if (n == 0)
    {
        printf("recv end");
        return -1;
    }

    // recv data
    if (recv(sockfd, packet, size, 0) <= 0)
    {
        perror("Failed to receive payload data");
        return -1;
    }

    *dataLength = size;
    return 0;
}

int sendSocketPacket(int sockfd, Socket_Packet_t *packet)
{
    int result = socket_send_payload(sockfd, packet,
                                     packet->header.packetLength);
    return result;
}

int recvSocketPacket(int sockfd, Socket_Packet_t *packet)
{
    size_t packet_size;

    // recv packet
    return socket_receive_payload(sockfd, (void *)packet, &packet_size);
}
