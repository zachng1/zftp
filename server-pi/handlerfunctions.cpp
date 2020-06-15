#include "handlerfunctions.h"

namespace zftp {
    namespace {
        std::vector<struct pollfd> createPollVector(std::unordered_map<int, User>& users, int selfPipeServerAlert, std::shared_timed_mutex& mutex) {
            std::vector<struct pollfd> pollingVector;

            struct pollfd serverAlert;
            serverAlert.fd = selfPipeServerAlert;
            serverAlert.events = POLLIN;
            serverAlert.revents = 0;
            pollingVector.push_back(serverAlert);
            
            std::shared_lock<std::shared_timed_mutex> lock(mutex);
            for (auto user: users) { 
                struct pollfd newFd;
                newFd.fd = user.first;
                newFd.events = POLLIN;
                newFd.revents = 0;
                pollingVector.push_back(newFd);
            }

            return pollingVector;
        }

        std::vector<struct pollfd> stringToPollVector(std::string listOfNewFds) {
            std::istringstream stream{listOfNewFds};
            std::vector<struct pollfd> newFdVector;
            std::string fd;

            while (std::getline(stream, fd, '|')) {
                struct pollfd newFd;
                newFd.fd = std::stoi(fd);
                newFd.events = POLLIN;
                newFd.revents = 0;
                newFdVector.push_back(newFd);
            }

            return newFdVector;
        }
    }
    void initHandling(bool& error, std::unordered_map<int, User>& users, std::shared_timed_mutex& mutex, int selfPipeServerAlert, int writeToDTP, int readFromDTP) {
        int readyCount;
        std::vector<struct pollfd> pollingVector;
        std::vector<struct pollfd> newFds;
        std::vector<struct pollfd> hangups;
        std::string newfdstring;

        pollingVector = createPollVector(users, selfPipeServerAlert, mutex);  

        while (true) {
            if ((readyCount = poll(&pollingVector[0], pollingVector.size(), -1) < 0)) {
                error = true;
                return;
            }
            for (auto pollfd: pollingVector) {
                //std::cout << pollfd.fd << ":" << pollfd.revents << std::endl;
                if (pollfd.revents & POLLERR || pollfd.revents & POLLHUP || pollfd.revents & POLLRDHUP) {
                    hangups.push_back(pollfd);
                }
                if (pollfd.revents & POLLIN) {
                    if (pollfd.fd == selfPipeServerAlert) {
                        int bytesRead = 0;
                        char buf[255];
                        //get any new fds from the listener (who sends it via a selfpipe)
                        while ((bytesRead = read(selfPipeServerAlert, buf, 255)) != 0) {
                            if (bytesRead < 0 && errno == EAGAIN) {
                               break;
                            }
                            else if (bytesRead < 0) {
                                error = true;
                                return;
                            }
                            newfdstring += std::string(buf);
                        }
                    }
                    else {
                        std::vector<std::string> commands;
                        commands = users.at(pollfd.fd).readCommand();
                        if (commands[0] == "DISCONNECTED") {
                            hangups.push_back(pollfd);
                        }
                        else if (commands.empty()) {
                            //handle other error
                        }
                        else {
                            for (auto arg: commands) {
                                std::cout << arg << std::endl;
                            }
                        }
                    }
                    pollfd.revents = 0;
                }
            }
            
            for (auto i: hangups) {
                shutdown(i.fd, SHUT_RDWR);
                close(i.fd);
                pollingVector.erase(std::find_if(pollingVector.begin(), pollingVector.end(), [i](auto j){
                    return i.fd == j.fd;
                    }));
            }
            hangups.clear();

            if (!newfdstring.empty()) {
                newFds = stringToPollVector(newfdstring);
                for (auto pollfd: newFds) {
                    pollingVector.push_back(pollfd);
                    users.at(pollfd.fd).sendResponse(220, std::string("Welcome to ZFTP!"));
                }
                newfdstring = "";
            }
        }
    }  
    void _202(User user) {
        user.sendResponse(202, "Not implemented.");
    }
    /*const std::unordered_map<std::string, void (*)(User)> commands{
        {std::string("USER"), _202},
        {std::string("PASS"), _202},
        {std::string("ACCT"), _202},
        {std::string("CWD"), _202},
        {std::string("CDUP"), _202},
        {std::string("SMNT"), _202},
        {std::string("REIN"), _202},
        {std::string("QUIT"), _202},
        {std::string("PORT"), _202},
        {std::string("PASV"), _202},
        {std::string("MODE"), _202},
        {std::string("TYPE"), _202},
        {std::string("STRU"), _202},
        {std::string("ALLO"), _202},
        {std::string("REST"), _202},
        {std::string("STOR"), _202},
        {std::string("STOU"), _202},
        {std::string("RETR"), _202},
        {std::string("LIST"), _202},
        {std::string("NLST"), _202},
        {std::string("APPE"), _202},
        {std::string("RNFR"), _202},
        {std::string("DELE"), _202},
        {std::string("RMD"), _202},
        {std::string("MKD"), _202},
        {std::string("PWD"), _202},
        {std::string("ABOR"), _202},
        {std::string("SYST"), _202},
        {std::string("STAT"), _202},
        {std::string("HELP"), _202},
        {std::string("SITE"), _202},
        {std::string("NOOP"), _202} 
    };*/
}