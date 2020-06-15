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
    // This blocks, call in a thread 
    void initHandling(bool& error, std::unordered_map<int, User>& users, std::shared_timed_mutex& mutex, int selfPipeServerAlert, int writeToDTP, int readFromDTP);    
    void _202(User u);
    //std::unordered_map<std::string, void (*)(User)> commands;    
}   

#endif