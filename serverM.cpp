#include "Socket.h"
#include "InetAddress.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <signal.h>
#include <sys/wait.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include "MessageStruct.h"
#include "UDPSocket.h"
#include "Util.h"
using namespace std;

Socket *serv_sock = NULL;
UdpSocket *udp_sock;

InetAddress serverSAddr("127.0.0.1", port_S);
InetAddress serverDAddr("127.0.0.1", port_D);
InetAddress serverUAddr("127.0.0.1", port_U);
std::map<string, string> credentials;
std::map<string, int> isMemberMap;

void signalHandler(int signum)
{
    // Handle SIGINT
    if (signum == SIGINT)
    {
        // std::cout << "Shutting down server..." << std::endl;
        if (serv_sock != nullptr)
        {
            serv_sock->socketClose(); // Close the server socket
            delete serv_sock;
        }
        if (udp_sock != nullptr)
        {
            udp_sock->socketClose(); // Close the udp socket
        }
        exit(signum); // Exit program
    }
}

void cleanUpChildProcess(int signalNumber)
{
    // Wait for all dead processes.
    // Use a non-blocking call to make sure this signal handler will not
    // block if a child was cleaned up in another part of the program.
    while (waitpid((pid_t)(-1), 0, WNOHANG) > 0)
    {
    }
}

std::map<string, std::string> read_credentials(const std::string &filename)
{
    std::map<std::string, std::string> credentials;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open())
    {
        std::cerr << "Error opening file" << std::endl;
        return credentials;
    }

    while (getline(file, line))
    {
        trim(line); // Trim whitespace and newlines from the line

        std::istringstream iss(line);
        std::string account, password;
        if (getline(iss, account, ',') && getline(iss, password))
        {
            trim(account);  // Trim the account
            trim(password); // Trim the password
            // cout << "Account:-" << account << "-Password:-" << password <<"-"<< endl;
            credentials[account] = password;
        }
    }
    file.close();
    return credentials;
}

int checkUserCredentials(string username, string password)
{
    // cout << "Checking user credentials for-" << username << "-and-" << password << endl;
    if (credentials.find(username) == credentials.end())
    {
        return USER_NAME_NOT_EXIT;
    }
    if (password.empty())
    {
        return USER_IS_GUEST;
    }
    if (credentials[username] != password)
    {
        return USER_PWD_NOT_MATCH;
    }
    return USER_IS_MEMBER;
}

int rxFromClient(Socket *sock, UdpSocket *udp_sock)
{
    Socket_Packet_t packet;
    int result = sock->recvSocketPacket(&packet);
    if (result == 0)
    {
        if (packet.header.type == USER_INFO_REQUEST)
        {

            User_Info_Request_t *request = (User_Info_Request_t *)packet.payload;
            User_Info_Response_t resp;
            if (request->status == USER_IS_GUEST)
            {

                cout << "The main server received the guest request for " << request->username << " using TCP over port " << getLocalPort(sock->getFd()) << ".The main server accepts " << request->username << " as a guest." << endl;

                isMemberMap[request->username] = USER_IS_GUEST;
                resp.status = USER_IS_GUEST;
                Socket_Packet_t packet;
                memset(&packet, 0, sizeof(packet));
                packet.header.type = USER_INFO_RESPONSE;
                packet.header.packetLength = sizeof(Socket_Packet_Header_t) + sizeof(User_Info_Response_t);
                memcpy(packet.payload, &resp, sizeof(User_Info_Response_t));
                sock->sendSocketPacket(&packet);

                cout << "The main server sent the guest response to the client" << endl;
            }
            else
            {
                cout << "The main server received the authentication for " << request->username << " using TCP over port " << getLocalPort(sock->getFd()) << endl;
                resp.status = checkUserCredentials(request->username, request->password);
                // update the isMemberMap
                isMemberMap[request->username] = resp.status;

                Socket_Packet_t packet;
                memset(&packet, 0, sizeof(packet));
                packet.header.type = USER_INFO_RESPONSE;
                packet.header.packetLength = sizeof(Socket_Packet_Header_t) + sizeof(User_Info_Response_t);
                memcpy(packet.payload, &resp, sizeof(User_Info_Response_t));
                sock->sendSocketPacket(&packet);

                cout << "The main server sent the authentication result to the client." << endl;
            }

            // cout << "true" << endl;
        }
        else if (packet.header.type == ROOM_INFO_REQUEST_AVAILABILITY)
        {
            Room_Info_Request_t *request = (Room_Info_Request_t *)packet.payload;
            cout << "The main server has received the availability request on Room " << request->room_code << " from " << request->username << " using TCP over port " << getLocalPort(sock->getFd()) << endl;
            if (request->room_code[0] == 'S')
            {
                cout << "The main server sent a request to Server S" << endl;
                // send to server S
                udp_sock->sendSocketPacket(&packet, &serverSAddr);
                // recv from server S
                udp_sock->recvSocketPacket(&packet, &serverSAddr);
                cout << "The main server received the response from Server S using UDP over port " << getLocalPort(udp_sock->getFd()) << endl;
                // send to client
                sock->sendSocketPacket(&packet);
            }
            else if (request->room_code[0] == 'D')
            {
                cout << "The main server sent a request to Server D" << endl;
                // send to server D
                udp_sock->sendSocketPacket(&packet, &serverDAddr);
                // recv from server S
                udp_sock->recvSocketPacket(&packet, &serverDAddr);
                cout << "The main server received the response from Server D using UDP over port " << getLocalPort(udp_sock->getFd()) << endl;
                // send to client
                sock->sendSocketPacket(&packet);
            }
            else if (request->room_code[0] == 'U')
            {
                cout << "The main server sent a request to Server U" << endl;
                // send to server U
                udp_sock->sendSocketPacket(&packet, &serverUAddr);
                // recv from server S
                udp_sock->recvSocketPacket(&packet, &serverUAddr);
                cout << "The main server received the response from Server U using UDP over port " << getLocalPort(udp_sock->getFd()) << endl;
                // send to client
                sock->sendSocketPacket(&packet);
            }
            else
            {
                Room_Info_Response_t response;
                response.available = ROOM_INFO_AVAILABLE_NOT_IN_SYSTEM;
                packet.header.type = ROOM_INFO_RESPONSE_AVAILABILITY;
                packet.header.packetLength = sizeof(Socket_Packet_Header_t) + sizeof(Room_Info_Response_t);
                memcpy(packet.payload, &response, sizeof(Room_Info_Response_t));
                sock->sendSocketPacket(&packet);
            }
            cout << "The main server sent the availability information to the client." << endl;

            // cout << "false" << endl;
        }
        else if (packet.header.type == ROOM_INFO_REQUEST_RESERVATION)
        {
            Room_Info_Request_t *request = (Room_Info_Request_t *)packet.payload;
            cout << "The main server has received the reservation request on Room " << request->room_code << " from " << request->username << " using TCP over port " << getLocalPort(sock->getFd()) << endl;
            // cout << isMemberMap[request->username] <<endl;
            if (isMemberMap[request->username] == USER_IS_MEMBER)
            {
                if (request->room_code[0] == 'S')
                {
                    cout << "The main server sent a request to Server S" << endl;
                    // send to server S
                    udp_sock->sendSocketPacket(&packet, &serverSAddr);
                    // recv from server S
                    udp_sock->recvSocketPacket(&packet, &serverSAddr);
                    cout << "The main server received the response from Server S using UDP over port " << getLocalPort(udp_sock->getFd()) << endl;
                    // send to client
                    sock->sendSocketPacket(&packet);
                }
                else if (request->room_code[0] == 'D')
                {
                    cout << "The main server sent a request to Server D" << endl;
                    // send to server D
                    udp_sock->sendSocketPacket(&packet, &serverDAddr);
                    // recv from server D
                    udp_sock->recvSocketPacket(&packet, &serverDAddr);
                    cout << "The main server received the response from Server D using UDP over port " << getLocalPort(udp_sock->getFd()) << endl;
                    // send to client
                    sock->sendSocketPacket(&packet);
                }
                else if (request->room_code[0] == 'U')
                {
                    cout << "The main server sent a request to Server U" << endl;
                    // send to server U
                    udp_sock->sendSocketPacket(&packet, &serverUAddr);
                    // recv from server U
                    udp_sock->recvSocketPacket(&packet, &serverUAddr);
                    cout << "The main server received the response from Server U using UDP over port " << getLocalPort(udp_sock->getFd()) << endl;
                    // send to client
                    sock->sendSocketPacket(&packet);
                }
                else
                {
                    Room_Info_Response_t response;
                    response.reserved = ROOM_INFO_RESERVED_NOT_IN_SYSTEM;
                    packet.header.type = ROOM_INFO_RESPONSE_RESERVATION;
                    packet.header.packetLength = sizeof(Socket_Packet_Header_t) + sizeof(Room_Info_Response_t);
                    memcpy(packet.payload, &response, sizeof(Room_Info_Response_t));
                    sock->sendSocketPacket(&packet);
                }
            }
            else
            {
                cout << request->username << " cannot make a reservation." << endl;
                cout << "The main server sent the error message to the client." << endl;
                // send error message to client
                Room_Info_Response_t response;
                response.reserved = ROOM_INFO_RESERVED_DENIED;
                memset(&packet, 0, sizeof(packet));
                packet.header.type = ROOM_INFO_RESPONSE_RESERVATION;
                packet.header.packetLength = sizeof(Socket_Packet_Header_t) + sizeof(Room_Info_Response_t);
                memcpy(packet.payload, &response, sizeof(Room_Info_Response_t));
                sock->sendSocketPacket(&packet);
            }
        }
        return 0;
    }
    else
    {
        // recv failed
        return -1;
    }
}

void rxRoomInfoFromServer(UdpSocket *udp_sock)
{
    // recv from serverD serverS serverU
    Socket_Packet_t packet;
    int i = 0;
    InetAddress recv_addr("127.0.0.1", 0);
    if (udp_sock->recvSocketPacket(&packet, &serverSAddr) == 0)
    {
        if (packet.header.type == SERVERS_SEND_ROOM_STATUS)
        {
            cout << "The main server has received the room status from Server S using UDP over port " << getLocalPort(udp_sock->getFd()) << endl;
        }
        // udp_sock->sendSocketPacket(&packet, &serverSAddr);
    }
    if (udp_sock->recvSocketPacket(&packet, &serverDAddr) == 0)
    {
        if (packet.header.type == SERVERD_SEND_ROOM_STATUS)
        {
            cout << "The main server has received the room status from Server D using UDP over port " << getLocalPort(udp_sock->getFd()) << endl;
        }
        // udp_sock->sendSocketPacket(&packet, &serverDAddr);
    }
    if (udp_sock->recvSocketPacket(&packet, &serverUAddr) == 0)
    {
        if (packet.header.type == SERVERU_SEND_ROOM_STATUS)
        {
            cout << "The main server has received the room status from Server U using UDP over port " << getLocalPort(udp_sock->getFd()) << endl;
        }
    }
}

int main()
{
    cout << "The main server is up and running." << endl;
    credentials = read_credentials("member.txt");
    signal(SIGINT, signalHandler);        // Set signal handler
    signal(SIGCHLD, cleanUpChildProcess); // Set signal handler for SIGCHLD
    // Create a UDP socket for communication with servers
    udp_sock = new UdpSocket();
    InetAddress udp_addr("127.0.0.1", port_M_UDP);
    udp_sock->socketBind(&udp_addr);

    // Create a server socket
    serv_sock = new Socket();
    InetAddress *serv_addr = new InetAddress("127.0.0.1", port_M_TCP);
    serv_sock->socketBind(serv_addr);
    serv_sock->socketListen();
    // first recv serverD serverS serverU
    rxRoomInfoFromServer(udp_sock);
    while (true)
    {
        InetAddress *clnt_addr = new InetAddress();
        Socket *clnt_sock = new Socket(serv_sock->socketAccept(clnt_addr));
        pid_t pid = fork();
        if (pid == 0)
        {                             // This is the child process
            serv_sock->socketClose(); // Close the listening socket in the child process

            // Communication with client
            // cout << "Child process created for client at " << endl;
            while (rxFromClient(clnt_sock, udp_sock) == 0)
                ;

            exit(0); // Terminate the child process
        }
        else if (pid > 0)
        {                             // This is the parent process
            clnt_sock->socketClose(); // Parent doesn't need this
        }
        else
        {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
    }
}