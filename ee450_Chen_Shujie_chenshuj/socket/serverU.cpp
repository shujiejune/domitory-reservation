#include "signal.h"
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

std::map<std::string, int> room_data_map = read_string_to_int_map("suite.txt");
UdpSocket serverSocket;

void signalHandler(int signum)
{
    // Handle SIGINT
    if (signum == SIGINT)
    {
        // std::cout << "Shutting down server..." << std::endl;
        if (serverSocket.getFd() != -1)
        {
            serverSocket.socketClose(); // Close the server socket
        }
        exit(signum); // Exit program
    }
}

int main()
{

    signal(SIGINT, signalHandler);                     // Set signal handler
    InetAddress serverAddr("127.0.0.1", port_U);       // Server will bind to port
    InetAddress serverm_addr("127.0.0.1", port_M_UDP); // if 0 ,it is ok
    InetAddress serverm_recv_addr("127.0.0.1", 0);     // if 0 ,it is ok
    serverSocket.socketBind(&serverAddr);
    cout << "The Server U is up and running using UDP on port " << getLocalPort(serverSocket.getFd()) << "." << endl;
    Socket_Packet_t packet;
    Server_Send_Room_Status_t send_room_status;
    packet.header.type = SERVERU_SEND_ROOM_STATUS;
    packet.header.packetLength = sizeof(Socket_Packet_Header_t) + sizeof(Server_Send_Room_Status_t);
    serverSocket.sendSocketPacket(&packet, &serverm_addr);
    cout << "The Server U has sent the room status to the main server." << endl;

    while (true)
    {
        // Client address will be filled in by recvfrom
        Socket_Packet_t packet;
        size_t receivedLength;

        if (serverSocket.recvSocketPacket(&packet, &serverm_addr) == 0)
        {
            // std::cout << "Received: " << packet.data << " from client" << std::endl;
            if (packet.header.type == ROOM_INFO_REQUEST_RESERVATION)
            {
                cout << "The Server U received a reservation request from the main server." << endl;

                Room_Info_Request_t request = *(Room_Info_Request_t *)packet.payload;
                Room_Info_Response_t response;
                string room_code(request.room_code);
                if (room_data_map.find(room_code) == room_data_map.end())
                {
                    cout << "Cannot make a reservation. Not able to find the room layout." << endl;
                    response.reserved = ROOM_INFO_RESERVED_NOT_IN_SYSTEM;
                }
                else if (room_data_map[room_code] > 0)
                {

                    room_data_map[room_code]--;
                    cout << "Successful reservation. The count of Room " << room_code << " is now " << room_data_map[room_code] << "."<< endl;
                    response.reserved = ROOM_INFO_RESERVED_SUCC;
                }
                else if (room_data_map[room_code] == 0)
                {
                    cout << "Cannot make a reservation. Room " << room_code << " is not available."<< endl;
                    response.reserved = ROOM_INFO_RESERVED_ZERO;
                }
                packet.header.type = ROOM_INFO_RESPONSE_RESERVATION;
                packet.header.packetLength = sizeof(Socket_Packet_Header_t) + sizeof(Room_Info_Response_t);
                memcpy(packet.payload, &response, sizeof(Room_Info_Response_t));
                serverSocket.sendSocketPacket(&packet, &serverm_addr);
                if (response.reserved == ROOM_INFO_RESERVED_SUCC)
                {
                    cout << "The Server U finished sending the response and the updated room status to the main server."<<endl;
                }
                else
                {
                    cout << "The Server U finished sending the response to the main server." << endl;
                }
            }
            else if (packet.header.type == ROOM_INFO_REQUEST_AVAILABILITY)
            {
                cout << "The Server U received an availability request from the main server." << endl;

                Room_Info_Request_t request = *(Room_Info_Request_t *)packet.payload;
                Room_Info_Response_t response;
                string room_code(request.room_code);
                // cout <<"--"<< room_code <<"--"<< endl;
                if (room_data_map.find(room_code) == room_data_map.end())
                {
                    cout << "Not able to find the room layout." << endl;
                    response.available = ROOM_INFO_AVAILABLE_NOT_IN_SYSTEM;
                }
                else if (room_data_map[room_code] > 0)
                {
                    cout << "Room " << room_code << " is available." << endl;
                    response.available = ROOM_INFO_IS_AVAILABLE;
                }
                else
                {
                    cout << "Room " << room_code << " is not available." << endl;
                    Room_Info_Response_t response;
                    response.available = ROOM_INFO_NOT_AVAILABLE;
                }
                packet.header.type = ROOM_INFO_RESPONSE_AVAILABILITY;
                packet.header.packetLength = sizeof(Socket_Packet_Header_t) + sizeof(Room_Info_Response_t);
                memcpy(packet.payload, &response, sizeof(Room_Info_Response_t));
                serverSocket.sendSocketPacket(&packet, &serverm_addr);
                cout << "The Server U finished sending the response to the main server." << endl;
            }
        }
        // Echo the message back to the client
        // serverSocket.socket_send_payload(&packet, receivedLength, &serverm_addr);
    }
}
