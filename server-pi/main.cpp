#include <iostream>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <shared_mutex>

//C Socket and Process headers:
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <fcntl.h>

#include "handlerfunctions.h"


int main(int argc, char * argv[]) {
    int writeToDTP = std::stoi(std::string(argv[1]));
    int readFromDTP = std::stoi(std::string(argv[2]));
    
    std::unordered_map<int, zftp::User> globalClientsList;
    std::shared_timed_mutex globalClientsListMutex;
    
    int serverPipe[2];
    if (pipe(serverPipe) != 0) {
        std::cerr << "Could not initialise self-pipe." << std::endl;
    }
    int flags = fcntl(serverPipe[1], F_GETFL, 0);
    fcntl(serverPipe[1], F_SETFL, flags | O_NONBLOCK);
    flags = fcntl(serverPipe[0], F_GETFL, 0);
    fcntl(serverPipe[0], F_SETFL, flags | O_NONBLOCK);
    
    //begin the thread that will poll existing clients
    //and process commands and responses.
    bool handlerError = false;
    std::thread handlerThread{&zftp::initHandling,
    std::ref(handlerError),
    std::ref(globalClientsList),
    std::ref(globalClientsListMutex),
    serverPipe[0], writeToDTP, readFromDTP};

    
    zftp::Listener listener;
    

    //Read will block until DTP sends a single byte
    //Ensure DTP is ready to handle requests before
    //beginning to listen for connections
    char readyBuf[1];
    if (read(readFromDTP, readyBuf, 1) != 1) {
        std::cerr << "Error reading from DTP" << std::endl;
    }
    std::cout << "DTP ready" << std::endl;

    listener.addListener(serverPipe[1], globalClientsList, globalClientsListMutex);
    
    int errorCount = 0;
    std::exception_ptr listenException;
    while (true) {
        while ((listenException = listener.getThreadError()) != nullptr) {
            errorCount += 10;
            //threads are guaranteed to exit on error, so restarting
            //is safe
            try {std::rethrow_exception(listenException);}
            catch (const std::runtime_error &e) {
                std::cout << "Error: " << e.what() << "Restarting listening thread." << std::endl;
            }
            listener.addListener(serverPipe[1], globalClientsList, globalClientsListMutex);
        }
        if (handlerError) {
            errorCount += 10;
            std::cout << "Error; restarting handler thread." << std::endl;
            handlerError = false;
            std::thread handlerThread{&zftp::initHandling,
            std::ref(handlerError),
            std::ref(globalClientsList),
            std::ref(globalClientsListMutex),
            serverPipe[0], writeToDTP, readFromDTP};
        }

        //This gives a tolerance of 10 errors for every 100 loops
        //If things are going horribly wrong it's time to shutdown the server
        if (errorCount > 100) {
            std::cerr << "Error threshold reached, shutting down." << std::endl;
            break;
        }
        else if (errorCount > 0) errorCount--;

    }
    std::cerr <<"EXIT"<<std::endl;

    if (handlerThread.joinable()) {
        handlerThread.join();
    }

    //signal parent

    return 0;
}