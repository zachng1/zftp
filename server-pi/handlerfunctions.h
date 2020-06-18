#ifndef __HANDLER_H
#define __HANDLER_H

#include <unordered_map>
#include <shared_mutex>
#include <vector>
#include <sstream>
#include <algorithm>

//C Socket headers:
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <unistd.h>
#include <poll.h>

#include "listener.h"
#include "user.h"

namespace zftp {
    //Main handling loop function. Handles polling existing users and calling the correct method when a command comes in.
    //This blocks, call in a thread.
    void initHandling(bool& error, std::unordered_map<int, User>& users, std::shared_timed_mutex& mutex, int selfPipeServerAlert, int writeToDTP, int readFromDTP);    
    void UNIMPLEMENTED_ERROR(std::vector<std::string>, User u);

    //Maps command names to functions for convenience.
    extern const std::unordered_map<std::string, void (*)(std::vector<std::string>, User)> commands;
    void USER(std::vector<std::string> args, User u);
}   

#endif