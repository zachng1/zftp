#ifndef __HANDLER_H
#define __HANDLER_H

#include <unordered_map>
#include <shared_mutex>
#include <vector>
#include <sstream>
#include <algorithm>
#include <limits>

//C Socket headers:
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <unistd.h>
#include <poll.h>

#include "listener.hpp"
#include "user.hpp"
#include "../utilities.hpp"

namespace zftp {
    //Main handling loop function. Handles polling existing users and calling the correct method when a command comes in.
    //This blocks, call in a thread.
    void initHandling(bool& error, std::unordered_map<int, User>& users, std::shared_timed_mutex& mutex, int selfPipeServerAlert, int writeToDTP, int readFromDTP);    

    //Maps command names to functions for convenience.
    extern const std::unordered_map<std::string, std::vector<std::string> (*)(std::vector<std::string>, User&)> commands;
    
    std::vector<std::string> UNIMPLEMENTED_ERROR(std::vector<std::string>, User& u);
    std::vector<std::string> USER(std::vector<std::string> args, User& u);
    std::vector<std::string> SYST(std::vector<std::string> args, User& u);
    std::vector<std::string> RETR(std::vector<std::string> args, User& u);
    std::vector<std::string> PORT(std::vector<std::string> args, User& u);
}   

#endif