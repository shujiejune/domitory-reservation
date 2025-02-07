#include "Socket.h"

Socket::Socket() : fd(-1)
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("socket create error");
    }
    int opt = 1;
    // Set socket to allow multiple bindings unless there are active connections
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(EXIT_FAILURE);
    }
}
Socket::Socket(int _fd) : fd(_fd)
{
    if (fd == -1)
    {
        perror("socket create error");
    }
}

Socket::~Socket()
{
    this->socketClose();
}

void Socket::socketBind(InetAddress *addr)
{
    if (bind(fd, (sockaddr *)&addr->addr, addr->addr_len))
    {
        perror("socket bind error");
    }
}

void Socket::socketListen()
{
    if (listen(fd, SOMAXCONN) == -1)
    {
        perror("socket listen error");
    }
}
void Socket::setnonblocking()
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int Socket::socketAccept(InetAddress *addr)
{
    int clnt_sockfd = accept(fd, (sockaddr *)&addr->addr, &addr->addr_len);
    if (clnt_sockfd == -1)
    {
        perror("socket accept error");
    }
    return clnt_sockfd;
}

int Socket::socketConnect(InetAddress *serv_addr)
{
    if (connect(fd, (struct sockaddr *)&(serv_addr->addr), sizeof(serv_addr->addr)) < 0)
    {
        close(fd);
        throw std::runtime_error("Connection failed with error: " + std::to_string(errno));
        return -1;
    }
    return 0;
}

int Socket::getFd()
{
    return fd;
}

void Socket::socketClose()
{
    if (fd != -1)
    {
        close(fd);
        fd = -1;
    }
}

int Socket::socket_send_payload(const void *payload, size_t dataLength)
{
    if (send(this->fd, &dataLength, sizeof(dataLength), 0) == -1)
    {
        perror("Failed to send payload size");
        return -1;
    }

    if (send(this->fd, payload, dataLength, 0) == -1)
    {
        perror("Failed to send payload data");
        return -1;
    }

    return 0;
}

int Socket::socket_receive_payload(void *packet, size_t *dataLength)
{
    // recv packet length
    size_t size;
    int n = recv(this->fd, &size, sizeof(size), 0);
    if (n < 0)
    {
        perror("Failed to receive payload size");
        return -1;
    }
    else if (n == 0)
    {
        // printf("recv end");
        return -1;
    }

    // recv data
    if (recv(this->fd, packet, size, 0) <= 0)
    {
        perror("Failed to receive payload data");
        return -1;
    }

    *dataLength = size;
    return 0;
}

int Socket::sendSocketPacket(Socket_Packet_t *packet)
{
    int result = socket_send_payload(packet,
                                     packet->header.packetLength);
    return result;
}

int Socket::recvSocketPacket(Socket_Packet_t *packet)
{
    size_t packet_size;

    // recv packet
    return socket_receive_payload((void *)packet, &packet_size);
}