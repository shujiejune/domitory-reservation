#ifndef MESSAGESTRUCT_H
#define MESSAGESTRUCT_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <string>

using namespace std;

typedef enum
{
    USER_INFO_REQUEST,
    USER_INFO_RESPONSE,
    USER_NAME_NOT_EXIT,
    USER_PWD_NOT_MATCH,
    USER_IS_MEMBER,
    USER_IS_GUEST,
    ROOM_INFO_REQUEST_AVAILABILITY,
    ROOM_INFO_REQUEST_RESERVATION,
    ROOM_INFO_RESPONSE_AVAILABILITY,
    ROOM_INFO_RESPONSE_RESERVATION,

    // ROOM AVAILABLE
    ROOM_INFO_IS_AVAILABLE,
    ROOM_INFO_NOT_AVAILABLE,
    ROOM_INFO_AVAILABLE_NOT_IN_SYSTEM, // reserved or available not in system
    ROOM_INFO_ERROR,
    // ROOM RESERVED
    ROOM_INFO_RESERVED_SUCC,
    ROOM_INFO_RESERVED_ZERO, // If the count of the room is 0
    ROOM_INFO_RESERVED_NOT_IN_SYSTEM,
    ROOM_INFO_RESERVED_DENIED,
    // serverS init information
    SERVERS_SEND_ROOM_STATUS,
    // serverD init information
    SERVERD_SEND_ROOM_STATUS,
    // serverU init information
    SERVERU_SEND_ROOM_STATUS,
    SUCC,
} Message_Type_t;

typedef struct
{
    size_t packetLength;
    int type;
} Socket_Packet_Header_t;

typedef struct
{
    Socket_Packet_Header_t header;
    char payload[2048];
} Socket_Packet_t;

typedef struct
{
    char username[128];
    char password[128];
    int status;
} User_Info_Request_t;

typedef struct
{
    int status;
} User_Info_Response_t;

typedef struct
{
    char username[128];
    char room_code[128];
} Room_Info_Request_t;

typedef struct
{
    int available;
    int reserved;
} Room_Info_Response_t;

typedef struct{
    int room_status;
}Server_Send_Room_Status_t;

#define offset 574
#define port_M_UDP 44000 + offset
#define port_M_TCP 45000 + offset
#define port_D 42000 + offset
#define port_S 41000 + offset
#define port_U 43000 + offset


int sendSocketPacket(int sockfd, Socket_Packet_t *packet);

int recvSocketPacket(int sockfd, Socket_Packet_t *packet);

int socket_send_payload(int sockfd, const void *payload, size_t payload_size);

int socket_receive_payload(int sockfd, void *packet, size_t *dataLength);

#endif // MESSAGE_STRUCT
