#ifndef SOCKET_H
#define SOCKET_H
#include "MessageStruct.h"
#include "InetAddress.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
class InetAddress;
class Socket
{
private:
    int fd;

public:
    Socket();
    Socket(int);
    ~Socket();

    void socketBind(InetAddress *);
    void socketListen();
    void setnonblocking();

    int socketAccept(InetAddress *);
    int socketConnect(InetAddress *);

    int getFd();
    void socketClose();
    int sendSocketPacket(Socket_Packet_t *packet);

    int recvSocketPacket(Socket_Packet_t *packet);

    int socket_send_payload(const void *payload, size_t payload_size);

    int socket_receive_payload(void *packet, size_t *dataLength);
};
#endif