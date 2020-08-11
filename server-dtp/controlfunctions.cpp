#include "controlfunctions.hpp"

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

    //this splits a list of commands (separated on '|')
    //then subsequently each command into its various
    //args (separated on ':') -- 
    //as defined in "PI->DTP command formats.txt"
    std::vector<std::string> commands, curCommand;
    commands = ZUtil::stringSplit(readIn, "|");
    std::string arg;
    for (auto command: commands) {
        curCommand = ZUtil::stringSplit(command, ":");
        result.push_back(curCommand);
    }
    return result;

}

bool processCommand(std::vector<std::string> command, 
std::unordered_map<int, std::unique_ptr<zftp::DataConnection>>& connections, 
std::vector<struct pollfd>& pollfds,
std::unordered_map<int, int>& fdToID) {
    
    int newConnectionFd = -1;
    struct pollfd newPollFd;
    //this needs special treatment, as we start listening then get a connection much later
    if (command[1].compare("P") == 0) {
        //do something wacky
        newConnectionFd = zftp::getPassiveConnectionFd(command);
        return false;
    }
    else {
        newConnectionFd = zftp::getActiveConnectionFd(command);
        if (newConnectionFd == -1) return false;

        newPollFd.fd = newConnectionFd;
        if (command[2].compare("D") == 0) {
            connections[newConnectionFd] = std::unique_ptr<zftp::DataConnection>
            (new zftp::DownloadConnection(newConnectionFd, command[1]));

            newPollFd.events = POLLOUT;
            
        }
        else if (command[2].compare("U") == 0) {
            connections[newConnectionFd] = std::unique_ptr<zftp::DataConnection> 
            (new zftp::UploadConnection(newConnectionFd, command[1]));

            newPollFd.events = POLLIN;
        }
        else return false;
        newPollFd.revents = 0;
        pollfds.push_back(newPollFd);
        fdToID[newPollFd.fd] = std::stoi(command[0]);
        return true;
    }
}

void processPollFd(struct pollfd pollfd, 
std::unordered_map<int, int>& fdToID,
std::vector<std::vector<std::string>>& commandsList,
std::unordered_map<int, std::unique_ptr<zftp::DataConnection>>& connections,
std::vector<struct pollfd>& completedConnections,
int readFromPi, int writeToPi) {
    if (pollfd.revents & POLLIN && pollfd.fd == readFromPi) {
        commandsList = parsePICommands(readFromPi);      
    }
    else if (pollfd.revents & POLLIN) {
        if (connections[pollfd.fd]->transferFile(255) == 0) {
            shutdown(pollfd.fd, SHUT_RDWR);
            close(pollfd.fd);                    
            connections.erase(pollfd.fd);
            completedConnections.push_back(pollfd);
        }
    }
    else if (pollfd.revents & POLLOUT) {
        int wrote;
        if ((wrote = connections[pollfd.fd]->transferFile(255)) <= 0) {
            std::cout << "Close connection" << std::endl;
            if (errno == EAGAIN);
            else {
                shutdown(pollfd.fd, SHUT_RDWR);
                close(pollfd.fd);
                connections.erase(pollfd.fd);
                completedConnections.push_back(pollfd);
                if (wrote == 0) {
                    std::string responseToPi = std::to_string(fdToID[pollfd.fd]) + "|";
                    write(writeToPi, responseToPi.c_str(), responseToPi.size());
                    fdToID.erase(pollfd.fd);
                }
            }
        }
    }
    pollfd.revents = 0;
}