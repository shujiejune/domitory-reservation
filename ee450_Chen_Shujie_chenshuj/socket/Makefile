

# server:
# 	g++ Util.cpp UDPSocket.cpp client.cpp connection.cpp InetAddress.cpp Socket.cpp MessageStruct.cpp -o client && \
# 	g++ Util.cpp UDPSocket.cpp InetAddress.cpp Socket.cpp MessageStruct.cpp serverM.cpp  -o server && \
# 	g++ Util.cpp UDPSocket.cpp InetAddress.cpp Socket.cpp MessageStruct.cpp serverS.cpp -o serverS
# clean:
# 	rm server && rm client

# Compiler to use
CC=g++

# Compiler flags, for example -g for debugging, -O2 for release
CFLAGS=-g -std=c++11

# Linker flags
LDFLAGS=

# Common object files shared between executables
COMMON_OBJS=Util.o UDPSocket.o InetAddress.o Socket.o MessageStruct.o

# Object files for each specific executable
OBJS=serverM.o serverS.o serverD.o serverU.o client.o
# OBJS=serverM.o serverS.o client.o

# Executables
EXECS=serverM serverS serverD serverU client
# EXECS=serverM serverS client

all: $(EXECS)
	rm -f $(OBJS) $(COMMON_OBJS) # Add this line to delete all object files after building

serverM: serverM.o $(COMMON_OBJS)
	$(CC) $(LDFLAGS) -o serverM serverM.o $(COMMON_OBJS)

serverS: serverS.o $(COMMON_OBJS)
	$(CC) $(LDFLAGS) -o serverS serverS.o $(COMMON_OBJS)

serverD: serverD.o $(COMMON_OBJS)
	$(CC) $(LDFLAGS) -o serverD serverD.o $(COMMON_OBJS)

serverU: serverU.o $(COMMON_OBJS)
	$(CC) $(LDFLAGS) -o serverU serverU.o $(COMMON_OBJS)

client: client.o $(COMMON_OBJS)
	$(CC) $(LDFLAGS) -o client client.o $(COMMON_OBJS)

# Generic rule for compiling C++ code
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(COMMON_OBJS) $(EXECS)

.PHONY: all clean