#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "InetAddress.h"
#include "MessageStruct.h"
#include "UDPSocket.h"

UdpSocket::UdpSocket() : fd(-1)
{
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1)
    {
        perror("socket create error");
    }
}

UdpSocket::UdpSocket(int _fd) : fd(_fd)
{
    if (fd == -1)
    {
        perror("socket create error");
    }
}

UdpSocket::~UdpSocket()
{
    this->socketClose();
}

void UdpSocket::socketBind(InetAddress *addr)
{
    if (bind(fd, (struct sockaddr *)&addr->addr, addr->addr_len) == -1)
    {
        perror("socket bind error");
    }
}

void UdpSocket::setNonBlocking()
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int UdpSocket::sendSocketPacket(Socket_Packet_t *packet, InetAddress *destAddr)
{
    return socket_send_payload(packet, packet->header.packetLength, destAddr);
}

int UdpSocket::recvSocketPacket(Socket_Packet_t *packet, InetAddress *srcAddr)
{
    size_t packet_size;
    return socket_receive_payload((void *)packet, &packet_size, srcAddr);
}

int UdpSocket::socket_send_payload(const void *payload, size_t payload_size, InetAddress *destAddr)
{
    ssize_t sentBytes = sendto(fd, payload, payload_size, 0,
                               (struct sockaddr *)&destAddr->addr, destAddr->addr_len);
    if (sentBytes == -1)
    {
        perror("socket send to error");
        return -1;
    }
    return 0;
}

int UdpSocket::socket_receive_payload(void *packet, size_t *dataLength, InetAddress *srcAddr)
{
    while (true)
    {
        socklen_t fromLen = srcAddr->addr_len;
        ssize_t recvBytes = recvfrom(fd, packet, *dataLength, 0,
                                     (struct sockaddr *)&srcAddr->addr, &fromLen);
        if (recvBytes == -1)
        {
            perror("socket receive from error");
            return -1;
        }else{
            *dataLength = recvBytes;
            break;
        }
    }
     // update the received data length
    return 0;
}

int UdpSocket::getFd() const
{
    return fd;
}

void UdpSocket::socketClose()
{
    if (fd != -1)
    {
        close(fd);
        fd = -1;
    }
}
