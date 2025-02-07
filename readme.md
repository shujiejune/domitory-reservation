**Full Name**: Shujie Chen

**Student ID**: 7181302574

In this project, I made a simple dormitory booking system that supports 2 types of users (member and guest) to execute 2 types of operations (checking, for both, and reservation, for members only) to 3 types of dormitory. I completed all functions:

- Util.h and Util.cpp provide the general functions for the socket programming.
- MessageStruct.h and MessageStruct.cpp present the packet structure and some packet type.
- InetAddress.h and InetAddress.cpp contain the functions for managing IP address.
- Socket.h and Socket.cpp provide the functions for creating and managing a TCP socket, e.g. binding, listening, connecting, closing, sending and receiving packets.
- UDPSocket.h and UDPSocket.cpp provide the functions for creating and managing a UDP socket, e.g. binding, listening, connecting, closing, sending and receiving packets.
- client.cpp  provides the client function.
- serverM.cpp  provides the main server function to communicate with serverS/D/U for multiple clients.
- serverS.cpp  provides the querys and reservations function for  Single Rooms.
- serverD.cpp  provides the querys and reservations function for  Double Rooms.
- serverU.cpp  provides the querys and reservations function for  Suites.

All messages are encapsulated as Socket_Packet_t(in MessageStruct.h) for sending and receiving.

**Idiosyncrasy**: When running serverM/S/D/U, sometimes you may see "socket receive from error: Bad address" on screen. On client side, the problem presents as stagnation after message "<username> sent an/a <availability/reservation> request to the main server." In this case, please press "Ctrl+C" in every terminal to exit the program and run again.