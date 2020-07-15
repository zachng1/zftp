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

//Function declarations

std::vector<std::vector<std::string>> parsePICommands(int readFromPi);

bool processCommand(std::vector<std::string> command, 
std::unordered_map<int, zftp::DataConnection>& connections, 
std::vector<struct pollfd>& pollfds);

void processPollFd(struct pollfd pollfd, 
std::vector<std::vector<std::string>>& commandsList,
std::unordered_map<int, zftp::DataConnection>& connections,
std::vector<struct pollfd>& completedConnections,
int readFromPi);

int main(int argc, char ** argv) {
    int writeToPi = std::stoi(std::string(argv[1]));
    int readFromPi = std::stoi(std::string(argv[2]));
    
    std::vector<struct pollfd> pollfds;
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
    std::unordered_map<int, zftp::DataConnection> connections;
    std::vector<struct pollfd> completedConnections;
    try {
    while (poll(&pollfds[0], pollfds.size(), -1)) {
        for (auto pollfd: pollfds) {
            processPollFd(pollfd, commandsList, connections, completedConnections, readFromPi);
        }
        for (auto command: commandsList) {
            if (!processCommand(command, connections, pollfds)) {
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
    }
    }
    catch (std::runtime_error e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}

//Call once poll signals there is data to read.
std::vector<std::vector<std::string>> parsePICommands(int readFromPi) {
    char buf[255]{};
    std::string readIn;
    unsigned long end;
    int bytesRead;
    std::vector<std::vector<std::string>> result;

    errno = 0;

    while ((bytesRead = read(readFromPi, buf, 255)) != -1) {
        buf[bytesRead] = '\0';
        readIn += std::string(buf);
    }
    
    //error conditions --> return empty vector
    if (errno != EWOULDBLOCK || errno != EAGAIN) return result;
    else if ((end = readIn.find_last_of('|')) == readIn.npos) return result;
    
    readIn = readIn.substr(0, end);
    std::cout << readIn << std::endl;

    //this splits a list of commands (separated on '|')
    //then subsequently each command into its various
    //args (separated on ':') -- 
    //as defined in "PI->DTP command formats.txt"
    std::vector<std::string> commands, curCommand;
    std::string command, arg;
    std::istringstream stream{readIn};
    while (std::getline(stream, command, '|')) {
        commands.push_back(command);
    }
    for (auto command: commands) {
        stream = std::istringstream{command};
        while (std::getline(stream, arg, ':')) {
            curCommand.push_back(arg);
        }
        result.push_back(curCommand);
    }

    return result;

}

bool processCommand(std::vector<std::string> command, 
std::unordered_map<int, zftp::DataConnection>& connections, 
std::vector<struct pollfd>& pollfds) {
    int newConnectionFd = -1;
    struct pollfd newPollFd;
    if (command[2].compare("A") == 0) {
        newConnectionFd = zftp::getActiveConnectionFd(command);
    }
    else if (command[2].compare("P") == 0) {
        newConnectionFd = zftp::getPassiveConnectionFd(command);
    }
    if (newConnectionFd == -1) return false;

    newPollFd.fd = newConnectionFd;
    if (command[3].compare("D") == 0) {
        zftp::DownloadConnection newDownload(newConnectionFd, command[1]);
        connections[newConnectionFd] = newDownload; 
        newPollFd.events = POLLOUT;
        
    }
    else if (command[3].compare("U") == 0) {
        zftp::UploadConnection newUpload(newConnectionFd, command[1]);
        connections[newConnectionFd] = newUpload;
        newPollFd.events = POLLIN;
    }
    else return false;
    newPollFd.revents = 0;
    pollfds.push_back(newPollFd);
    return true;
}

void processPollFd(struct pollfd pollfd, 
std::vector<std::vector<std::string>>& commandsList,
std::unordered_map<int, zftp::DataConnection>& connections,
std::vector<struct pollfd>& completedConnections,
int readFromPi) {
    if (pollfd.revents & POLLIN && pollfd.fd == readFromPi) {
                commandsList = parsePICommands(readFromPi);
                std::cout << "PARSED COMMANDS ONCE" << std::endl;                
    }
    else if (pollfd.revents & POLLIN) {
        if (connections[pollfd.fd].transferFile(255) == 0) {
            shutdown(pollfd.fd, SHUT_RDWR);
            close(pollfd.fd);                    
            connections.erase(pollfd.fd);
            completedConnections.push_back(pollfd);
        }
    }
    else if (pollfd.revents & POLLOUT) {
        if (connections[pollfd.fd].transferFile(255) == 0) {
            shutdown(pollfd.fd, SHUT_RDWR);
            close(pollfd.fd);
            connections.erase(pollfd.fd);
            completedConnections.push_back(pollfd);
        }
    }
    pollfd.revents = 0;
}