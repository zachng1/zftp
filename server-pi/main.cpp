#include <iostream>
#include <vector>
#include <shared_mutex>

//C Socket headers:
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 

#include "listener.h"


int main() {
    std::vector<int> globalClientSockets;
    std::shared_timed_mutex globalSocketsMutex;

    //Listener init involves setting up an binding to an address
    //this makes sure we catch any failures: of course it also means we have 
    //to remember to delete listener before returning...
    Listener * listener;
    try {
        listener = new Listener;
    }
    catch (const std::runtime_error & e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    listener->addListener(globalClientSockets, globalSocketsMutex);
    

    
    while (true) {
        while (listener->getThreadError() != nullptr) {
            //some logic to restart thread, or if this gets called
            //too much just exit
            std::cout << "Caught some error" << std::endl;
        }
    }
    //in event loop -- call getThreadError() on listener to check for nullptr

    //we want a new listener which will look only for incoming connections,
    //before creating a User object from any handle that then gets passed 
    //to a Handler object which will poll all Users objects
    //Listener poll and handler Poll get separate threads
    delete listener;
    return 0;
}