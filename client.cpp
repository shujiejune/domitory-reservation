#include "Socket.h"
#include "InetAddress.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <cstdlib>
#include <signal.h>
#include "MessageStruct.h"
#include "Util.h"
using namespace std;
User_Info_Request_t user_info;

// connect to main server
InetAddress serverM_addr("127.0.0.1", port_M_TCP);

Socket my_sock = Socket();

void offsetCharacters(char *str)
{
    if (str == nullptr)
        return; // Guard against null pointer

    for (int i = 0; str[i] != '\0'; ++i)
    {
        if (isalpha(str[i]))
        { // Check if it's a letter
            char offset_base = (isupper(str[i]) ? 'A' : 'a');
            // Shift within 'A' to 'Z' or 'a' to 'z'
            str[i] = (str[i] - offset_base + 3) % 26 + offset_base;
        }
        else if (isdigit(str[i]))
        { // Check if it's a digit
            // Shift within '0' to '9'
            str[i] = (str[i] - '0' + 3) % 10 + '0';
        }
        // Non-alphanumeric characters are not changed
    }
}

// int sendToServer(int sockfd, const char *message)
// {
//     Socket_Packet_t packet;
//     memset(&packet, 0, sizeof(packet));
//     packet.header.packetLength = sizeof(Socket_Packet_Header_t) + strlen(message);
//     strncpy(packet.payload, message, sizeof(packet.payload) - 1);

//     return sendSocketPacket(sockfd, &packet);
// }

bool processLogin(int sockfd)
{
    // process input
    cout << "Please enter the username: ";
    string username,password;
    getline(cin, username);
    cout << "Please enter the password: ";
    getline(cin, password);
    strcpy(user_info.username, username.c_str());
    strcpy(user_info.password, password.c_str());
    offsetCharacters(user_info.password);
    offsetCharacters(user_info.username);
    // cout << "--" << user_info.username << "--" << endl;
    // cout << "--" << user_info.password << "--" << endl;
    // request MServer
    if (user_info.password[0] == '\0')
    {
        user_info.status = USER_IS_GUEST;
        cout << user_info.username << " sent a guest request to the main server using TCP over port "<< getLocalPort(sockfd) << "." << endl;
    }
    else
    {
        user_info.status = USER_IS_MEMBER;
        cout << user_info.username << " sent an authentication request to the main server." << endl;
    }
    Socket_Packet_t packet;
    memset(&packet, 0, sizeof(packet));
    packet.header.type = USER_INFO_REQUEST;
    packet.header.packetLength = sizeof(Socket_Packet_Header_t) + sizeof(User_Info_Request_t);
    memcpy(packet.payload, &user_info, sizeof(User_Info_Request_t));
    sendSocketPacket(sockfd, &packet);

    // receive response
    Socket_Packet_t response;
    int result = recvSocketPacket(sockfd, &response);
    bool isLoginSuccess = 0;
    if (result == 0)
    {
        if (response.header.type == USER_INFO_RESPONSE)
        {
            User_Info_Response_t *user_info_response = (User_Info_Response_t *)response.payload;
            if (user_info_response->status == USER_NAME_NOT_EXIST)
            {
                cout << "Failed login: Username does not exist." << endl;
            }
            else if (user_info_response->status == USER_PWD_NOT_MATCH)
            {
                cout << "Failed login: Password does not match." << endl;
            }
            else if (user_info_response->status == USER_IS_GUEST)
            {
                cout << "Welcome guest " << user_info.username << "!" << endl;
                user_info.status = USER_IS_GUEST;
                isLoginSuccess = true;
            }
            else if (user_info_response->status == USER_IS_MEMBER)
            {
                user_info.status = USER_IS_MEMBER;
                cout << "Welcome member " << user_info.username << "!" << endl;
                isLoginSuccess = true;
            }
        }
    }
    return isLoginSuccess;
}

void processChat(int sockfd)
{
    Room_Info_Request_t room_info;
    strcpy(room_info.username, user_info.username);

    while (true)
    {
        cout << "Please enter the room code: ";
        cin >> room_info.room_code;
        cout << "Would you like to search for the availability or make a reservation?";
        cout << "(Enter \"Availability\" to search for the availability or Enter \"Reservation\" to make a reservation ): ";
        string choice;
        cin >> choice;
        //

        Socket_Packet_t packet;
        memset(&packet, 0, sizeof(packet));
        if (choice == "Availability")
        {
            packet.header.type = ROOM_INFO_REQUEST_AVAILABILITY;
            cout << user_info.username << " sent an availability request to the main server." << endl;
        }
        else if (choice == "Reservation")
        {
            packet.header.type = ROOM_INFO_REQUEST_RESERVATION;
            cout << user_info.username << " sent an reservation request to the main server." << endl;
        }
        else
        {
            cout << "invalid choice" << endl;
            continue;
        }

        packet.header.packetLength = sizeof(Socket_Packet_Header_t) + sizeof(Room_Info_Request_t);
        memcpy(packet.payload, &room_info, sizeof(Room_Info_Request_t));
        sendSocketPacket(sockfd, &packet);

        // receive response
        // receive response
        Socket_Packet_t response;
        int result = recvSocketPacket(sockfd, &response);
        if (result == 0)
        {
            if (response.header.type == ROOM_INFO_RESPONSE_AVAILABILITY)
            {
                Room_Info_Response_t *room_info_response = (Room_Info_Response_t *)response.payload;
                if (room_info_response->available == ROOM_INFO_IS_AVAILABLE)
                {
                    cout << "The client received the response from the main server using TCP over port " << getLocalPort(sockfd) << "." << endl << "The requested room is available." << endl;
                    cout << endl
                         << "-----Start a new request-----" << endl;
                }
                else if (room_info_response->available == ROOM_INFO_NOT_AVAILABLE)
                {
                    cout << "The client received the response from the main server using TCP over port " << getLocalPort(sockfd) << "." << endl << "The requested room is not available." << endl;
                    cout << endl
                         << "-----Start a new request-----" << endl;
                }
                else if (room_info_response->available == ROOM_INFO_AVAILABLE_NOT_IN_SYSTEM)
                {
                    cout << "The client received the response from the main server using TCP over port " << getLocalPort(sockfd) << "." << endl << "Not able to find the room layout." << endl;
                    cout << endl
                         << "-----Start a new request-----" << endl;
                }
            }
            else if (response.header.type == ROOM_INFO_RESPONSE_RESERVATION)
            {
                Room_Info_Response_t *room_info_response = (Room_Info_Response_t *)response.payload;
                if (room_info_response->reserved == ROOM_INFO_RESERVED_SUCC)
                {
                    cout << "The client received the response from the main server using TCP over port " << getLocalPort(sockfd)<< "." << endl  << "Congratulation! The reservation for Room " << room_info.room_code << " has been made." << endl;
                    cout << endl
                         << "-----Start a new request-----" << endl;
                }
                else if (room_info_response->reserved == ROOM_INFO_RESERVED_ZERO)
                {
                    cout << "The client received the response from the main server using TCP over port " << getLocalPort(sockfd)<< "." << endl << "Sorry! The requested room is not available" << endl;
                    cout << endl
                         << "-----Start a new request-----" << endl;
                }
                else if (room_info_response->reserved == ROOM_INFO_RESERVED_NOT_IN_SYSTEM)
                {
                    cout << "The client received the response from the main server using TCP over port " << getLocalPort(sockfd)<< "." << endl  << "Oops! Not able to find the room." << endl;
                    cout << endl
                         << "-----Start a new request-----" << endl;
                }
                else if (room_info_response->reserved == ROOM_INFO_RESERVED_DENIED)
                {
                    cout << "Permission denied: Guest cannot make a reservation." << endl;
                }
            }
        }
        else
        {
            cout << "failed to receive response" << endl;
        }
    }
}

int main()
{
    my_sock.socketConnect(&serverM_addr);
    cout << "Client is up and running." << endl;
    while (processLogin(my_sock.getFd())==false);
    processChat(my_sock.getFd());
    // sendToServer(conn->getSocketFd(), "Hello, world!");
}