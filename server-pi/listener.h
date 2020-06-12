#ifndef __LISTENER_H
#define __LISTENER_H 

#include <iostream>
#include <stdexcept>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

//C Socket headers:
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <unistd.h>

#include "user.h"

namespace zftp {
    class Listener {
        public:
        Listener();
        ~Listener();
        Listener(const Listener& l) = delete;
        Listener& operator = (const Listener& l) = delete;

        std::vector<std::string> getAddresses();
        //The listener has to manage a thread anyway, so why not make it
        //scalable
        void addListener(int writePipe, std::unordered_map<int, User>& clientsList, std::shared_timed_mutex& mutex);
        std::exception_ptr getThreadError();
        

        private:
        void _blockingListen(int writePipe, std::unordered_map<int, User>& clientsList, std::shared_timed_mutex& mutex);
        
        int listenerSocket;
        std::shared_timed_mutex errorMutex;
        std::vector<std::thread> listeningThreads;
        std::queue<std::exception_ptr> threadErrors;
        

    };
}

#endif