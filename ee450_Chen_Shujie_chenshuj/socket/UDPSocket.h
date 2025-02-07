#ifndef UDPSOCKET_H
#define UDPSOCKET_H
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
class UdpSocket
{
public:
    UdpSocket();
    explicit UdpSocket(int _fd);
    ~UdpSocket();

    void socketBind(InetAddress *addr);
    void setNonBlocking();
    int sendSocketPacket(Socket_Packet_t *packet, InetAddress *destAddr);
    int recvSocketPacket(Socket_Packet_t *packet, InetAddress *srcAddr);
    int socket_send_payload(const void *payload, size_t payload_size, InetAddress *destAddr);
    int socket_receive_payload(void *packet, size_t *dataLength, InetAddress *srcAddr);
    int getFd() const;
    void socketClose();

private:
    int fd;
};
#endif // UDPSOCKET_H