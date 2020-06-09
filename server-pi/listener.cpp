#include "listener.h"

Listener::Listener() : socketHandle{-1}
{    
    int status;
    struct addrinfo hints{};
    struct addrinfo * servinfo;
    
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (status = getaddrinfo(NULL, "ftp", &hints, &servinfo) != 0) {
        throw std::runtime_error("Could not initialise address info struct.");
    }

    for (struct addrinfo * i = servinfo; i != nullptr; i = i->ai_next) {
        if ((socketHandle = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == -1) {
            continue;
        }

        if (bind(socketHandle, i->ai_addr, i->ai_addrlen) == -1) {
            continue;
        }
        break;
    }

    if (socketHandle == -1) {
        throw std::runtime_error("Could not either bind or initialise socket.");
    }
    else freeaddrinfo(servinfo); 

    if (listen(socketHandle, 500) == -1) {
        throw std::runtime_error("Could not listen on ftp port.");
    }
}

Listener::~Listener() {
    close(socketHandle);
    std::cout << "Listener destructor" << std::endl;
    for (auto& thread: listeningThreads){
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void Listener::addListener(std::vector<int>& sockets, std::shared_timed_mutex& mutex) {
    //error checking for threads will be done via getThreadError()
    //up to handler of Listener object to check this periodically
    listeningThreads.push_back(std::thread(&Listener::_blockingListen, this, std::ref(sockets), std::ref(mutex)));
}

std::exception_ptr Listener::getThreadError() {
    if (threadErrors.empty()) {
        return nullptr;
    }
    std::exception_ptr latestErr = threadErrors.back();
    threadErrors.pop();
    return latestErr;
}

void Listener::_blockingListen(std::vector<int>& sockets, std::shared_timed_mutex& mutex) {
    try {
        struct sockaddr_storage clientAddress{};
        socklen_t addrLength = sizeof(clientAddress);
        while (true) {
            int newClient;
            throw std::exception();
            std::cout << "Hello from new thread" << std::this_thread::get_id() << std::endl;
            if ((newClient = accept(socketHandle, (struct sockaddr *) &clientAddress, &addrLength)) == -1) {
                throw std::runtime_error("Could not accept incoming connection.");
            }
            else {
                std::lock_guard<std::shared_timed_mutex> lock(mutex);
                sockets.push_back(newClient);
            }
        }
    }
    catch (...) {
        threadErrors.push(std::current_exception());
    }
    
}