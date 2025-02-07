#include "Util.h"
// Helper function to trim whitespace from both ends of a string
void trim(std::string &str) {
    // Left trim
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    // Right trim
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), str.end());
}

std::map<std::string, int> read_string_to_int_map(const std::string& filename) {
    std::map<std::string, int> dataMap;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return dataMap;
    }

    while (getline(file, line)) {
        trim(line);  // Trim whitespace and newlines from the line

        std::istringstream iss(line);
        std::string key;
        std::string valueStr;
        int value;

        if (getline(iss, key, ',') && getline(iss, valueStr)) {
            trim(key);  // Trim the key
            trim(valueStr);  // Trim the value string
            value = std::stoi(valueStr);  // Convert value string to int
            dataMap[key] = value;
            // cout << "Key:-" << key << ", Value:-" << value << endl;
        }
    }

    file.close();
    return dataMap;
}
int getLocalPort(int sockfd)
{
    sockaddr_in localAddress;
    socklen_t addressLength = sizeof(localAddress);
    if (getsockname(sockfd, (struct sockaddr *)&localAddress, &addressLength) == -1)
    {
        std::cerr << "Error getting socket name" << std::endl;
        return -1;
    }
    return ntohs(localAddress.sin_port);
}