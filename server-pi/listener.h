#ifndef __LISTENER_H
#define __LISTENER_H 

#include <iostream>
#include <stdexcept>
#include <vector>
#include <queue>
#include <thread>
#include <shared_mutex>

//C Socket headers:
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <unistd.h>


class Listener {
    public:
    Listener();
    ~Listener();

    std::vector<std::string> getAddresses();
    void addListener(std::vector<int>& sockets, std::shared_timed_mutex& mutex);
    std::exception_ptr getThreadError();
    

    private:
    void _blockingListen(std::vector<int>& sockets, std::shared_timed_mutex& mutex);
    int socketHandle;
    std::vector<std::thread> listeningThreads;
    std::queue<std::exception_ptr> threadErrors;
    

};

#endif