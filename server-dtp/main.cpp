/* responsibilities:
    - listen for messages from PI (new data connection, expect read, expect write)
    - define a handling function for reading formatted messages
    - initiate new data connections (has a class)
    - read incoming data (process uploads)
    - write outgoing data (process downloads)
    use image mode (BINARY) for all transfers
*/  

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
#include "controlfunctions.hpp"
#include "../utilities.hpp"

//Function declarations

int main(int argc, char ** argv) {

    int writeToPi = std::stoi(std::string(argv[1]));
    int readFromPi = std::stoi(std::string(argv[2]));

    std::vector<struct pollfd> pollfds;
    std::unordered_map<int, int> fdToID;
    struct pollfd readFromPiPollfd;
    readFromPiPollfd.fd = readFromPi;
    readFromPiPollfd.events = POLLIN;
    readFromPiPollfd.revents = 0;
    pollfds.push_back(readFromPiPollfd);
    

    //signal to PI that we are ready to start sending and recieving data
    char readyBuf[1]{'A'};
    if (write(writeToPi, readyBuf, 1) < 0) {
        std::cout << "Error signalling PI" << std::endl;
        return 1;
    };

    std::vector<std::vector<std::string>> commandsList;
    std::unordered_map<int, std::unique_ptr<zftp::DataConnection>> connections;
    std::vector<struct pollfd> completedConnections;
    try {
    while (poll(&pollfds[0], pollfds.size(), -1)) {
        for (auto pair: pollfds) {
            processPollFd(pair, fdToID, commandsList, connections, completedConnections, readFromPi, writeToPi);
        }
        for (auto command: commandsList) {
            if (!processCommand(command, connections, pollfds, fdToID)) {
                std::cout << "Error with command" << std::endl;
                //write a message back to PI with id and error
                //something like send(writeToPi, "ERROR ON COMMAND: %d", command[0], 0000 etc.)
                //obviously fprint to the send buf first
            }      
        }
        for (auto completion: completedConnections) {
            pollfds.erase(std::find_if(pollfds.begin(), pollfds.end(), [completion](auto j){
                    return completion.fd == j.fd;
                    }));
        }
        completedConnections.clear();
        commandsList.clear();
    }
    }
    catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    catch (...) {
        std::cout << "Unknown exception" << std::endl;
    }
    
    std::cout << "Exit normal" << std::endl;
    return 0;
}

