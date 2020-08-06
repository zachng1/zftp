#ifndef __CONTROL_H
#define __CONTROL_H
#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <memory>

//C Socket headers:
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <unistd.h>
#include <errno.h>
#include <poll.h>

#include "dataconnection.hpp"
#include "../utilities.hpp"

std::vector<std::vector<std::string>> parsePICommands(int readFromPi);

bool processCommand(std::vector<std::string> command, 
std::unordered_map<int, std::unique_ptr<zftp::DataConnection>>& connections, 
std::vector<struct pollfd>& pollfds,
std::unordered_map<int, int>& fdToID);

void processPollFd(struct pollfd pollfd, 
std::unordered_map<int, int>& fdToID,
std::vector<std::vector<std::string>>& commandsList,
std::unordered_map<int, std::unique_ptr<zftp::DataConnection>>& connections,
std::vector<struct pollfd>& completedConnections,
int readFromPi, int writeToPi);

#endif