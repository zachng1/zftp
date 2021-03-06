#include "listener.hpp"

namespace zftp {
    Listener::Listener() : 
    listenerSocket{-1},
    errorMutex{}
    {    
        int status;
        struct addrinfo hints{};
        struct addrinfo * servinfo;
        
        hints.ai_family = AF_INET6;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        if ((status = getaddrinfo(NULL, "ftp", &hints, &servinfo)) != 0) {
            throw std::runtime_error("Could not initialise address info struct.");
        }

        for (struct addrinfo * i = servinfo; i != nullptr; i = i->ai_next) {
            if ((listenerSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == -1) {
                continue;
            }

            if (bind(listenerSocket, i->ai_addr, i->ai_addrlen) == -1) {
                continue;
            }
            break;
        }

        if (listenerSocket == -1) {
            throw std::runtime_error("Could not either bind or initialise socket.");
        }
        else freeaddrinfo(servinfo); 

        if (listen(listenerSocket, 500) == -1) {
            throw std::runtime_error("Could not listen on ftp port.");
        }
        struct sockaddr_in sin;
        socklen_t len = sizeof(sin);
        if (getsockname(listenerSocket, (struct sockaddr *)&sin, &len) == -1)
            perror("getsockname");
        else
            printf("port number %d\n", ntohs(sin.sin_port));
    }


    Listener::~Listener() {
        close(listenerSocket);
        for (auto& thread: listeningThreads){
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    //another option is for the Listener to maintain its
    //own mutex on the socket vector. But since the owner of the Listener object
    //has to check for thread errors manually, forcing them to pass in a mutex
    //is a good reminder that Listener handles threads
    void Listener::addListener(const int alertPipe, std::unordered_map<int, User>& clientsList, std::shared_timed_mutex& mutex) {
        listeningThreads.push_back(std::thread(&Listener::_blockingListen, this, alertPipe, std::ref(clientsList), std::ref(mutex)));
    }

    std::exception_ptr Listener::getThreadError() {
        if (std::shared_lock<std::shared_timed_mutex> lock(errorMutex); 
        threadErrors.empty()) {
            return nullptr;
        }
        std::scoped_lock<std::shared_timed_mutex> lock(errorMutex);
        std::exception_ptr latestErr = threadErrors.back();
        threadErrors.pop();
        return latestErr;
    }

    void Listener::_blockingListen(const int alertPipe, std::unordered_map<int, User>& clientsList, std::shared_timed_mutex& mutex) {
        try {
            std::cout << "New thread: " << std::this_thread::get_id() << std::endl;
            struct sockaddr_storage clientAddress{};
            socklen_t addrLength = sizeof(clientAddress);
            while (true) {
                int newSocket = -1;
                if ((newSocket = accept4(listenerSocket, (struct sockaddr *) &clientAddress, &addrLength, SOCK_NONBLOCK)) == -1) {
                    throw std::runtime_error("Could not accept an incoming connection.");
                }
                else {
                    std::cout << "New client: " << newSocket << std::endl;
                    std::scoped_lock<std::shared_timed_mutex> lock(mutex);
                    clientsList.emplace(newSocket, zftp::User(newSocket));
                    //then signal to the handler that it needs to update it's
                    //pollfd struct
                    char fdBuf[64]{};
                    snprintf(fdBuf, 64, "%d|", newSocket);
                    errno = 0;
                    if (write(alertPipe, fdBuf, strlen(fdBuf)) < 0) {
                        std::cout << errno << std::endl;
                        throw std::runtime_error("Could not send FD to handler.");
                    };
                }
            }
        }
        catch (...) {
            std::scoped_lock<std::shared_timed_mutex> lock(errorMutex);
            threadErrors.push(std::current_exception());
            return;
        }
        
    }
}