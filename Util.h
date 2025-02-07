#ifndef UTIL_H
#define UTIL_H

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
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <algorithm>
#include <cctype>


void trim(std::string &str);
std::map<std::string, int> read_string_to_int_map(const std::string &filename);
int getLocalPort(int sockfd);
#endif