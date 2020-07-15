#include "handlerfunctions.hpp"

namespace zftp {
    // These are a number of helper functions for the main init handling method
    namespace {
        std::vector<struct pollfd> createPollVector(std::unordered_map<int, User>& users, int selfPipeServerAlert, int readFromDTP, std::shared_timed_mutex& mutex) {
            std::vector<struct pollfd> pollingVector;

            struct pollfd serverAlert;
            serverAlert.fd = selfPipeServerAlert;
            serverAlert.events = POLLIN;
            serverAlert.revents = 0;
            pollingVector.push_back(serverAlert);

            struct pollfd DTPReadAlert;
            DTPReadAlert.fd = readFromDTP;
            DTPReadAlert.events = POLLIN;
            DTPReadAlert.revents = 0;
            pollingVector.push_back(DTPReadAlert);
            
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

        std::string processPipeAlert(int selfPipeServerAlert) {
            int bytesRead = 0;
            char buf[255]{};
            std::string newstring{};
            //get any new fds from the listener (who sends it via a selfpipe)
            while ((bytesRead = read(selfPipeServerAlert, buf, 255)) != 0) {
                if (bytesRead < 0 && errno == EAGAIN) {
                    break;
                }
                else if (bytesRead < 0) {
                    return "ERROR";
                }
                newstring += std::string(buf);
            }
            return newstring;
        }

        void processHangups(std::unordered_map<int, User>& users, std::vector<struct pollfd>& hangups, std::vector<struct pollfd>& pollingVector, std::shared_timed_mutex& mutex) {
            for (auto i: hangups) {
                shutdown(i.fd, SHUT_RDWR);
                close(i.fd);
                pollingVector.erase(std::find_if(pollingVector.begin(), pollingVector.end(), [i](auto j){
                    return i.fd == j.fd;
                    }));
                std::scoped_lock<std::shared_timed_mutex> lock(mutex);
                users.erase(i.fd);
            }
            hangups.clear();
        }

        int sendToDTP(std::vector<std::string> DTPCommand, int writeToDTP, int ID) {
            std::string command;
            command += std::to_string(ID);
            for (auto arg: DTPCommand) command += ":" + arg;
            command += "|";
            std::cout << "Sent command to DTP ONCE" << std::endl;
            int bytesWritten, total = 0;
            while (total < command.size()) {
                if ((bytesWritten = write(writeToDTP, command.c_str() + total, command.size() - total)) < 0) {
                    return -1;
                }
                total += bytesWritten;
            }
            return 0;
        }
    }

    void initHandling(bool& error, std::unordered_map<int, User>& users, std::shared_timed_mutex& usersMutex, int selfPipeServerAlert, int writeToDTP, int readFromDTP) {
        int readyCount, ID = 0;
        std::vector<struct pollfd> pollingVector;
        std::vector<struct pollfd> newFds;
        std::vector<struct pollfd> hangups;
        std::string newfdstring;

        pollingVector = createPollVector(users, selfPipeServerAlert, readFromDTP, usersMutex);  

        while (true) {
            if ((readyCount = poll(&pollingVector[0], pollingVector.size(), -1) < 0)) {
                error = true;
                return;
            }
            for (auto pollfd: pollingVector) {
                if (pollfd.revents & POLLERR || pollfd.revents & POLLHUP || pollfd.revents & POLLRDHUP) {
                    hangups.push_back(pollfd);
                }
                if (pollfd.revents & POLLIN && pollfd.fd == selfPipeServerAlert) {
                    if ((newfdstring = processPipeAlert(selfPipeServerAlert)) == "ERROR") {
                        error = true;
                        return;
                    }
                }
                else if (pollfd.revents & POLLIN && pollfd.fd == readFromDTP) {

                }
                else if (pollfd.revents & POLLIN) {
                    std::vector<std::string> message;
                    std::vector<std::string> DTPCommand;
                    std::shared_lock<std::shared_timed_mutex> lock(usersMutex);
                    User& user = users.at(pollfd.fd);
                    message = user.readMessage();
                    if (message[0] == "DISCONNECTED" || message.empty()) {
                        hangups.push_back(pollfd);
                    }
                    else {
                        try {
                            std::cout << user.getName() << ": " << message[0] << std::endl;
                            //use at instead of [] because commands is const
                            DTPCommand = zftp::commands.at(message[0])(message, user);
                        }
                        catch (std::out_of_range &e) {
                            user.sendResponse(500, "INTERNAL ERROR");
                        }
                    }
                    if (!DTPCommand.empty()) {
                        if (sendToDTP(DTPCommand, writeToDTP, ID) < 0) {
                            return;
                        }
                        DTPCommand.clear();
                        ID < std::numeric_limits<int>::max() ? ++ID : ID = 0;
                    }
                }
                pollfd.revents = 0;
            }
            processHangups(users, hangups, pollingVector, usersMutex);

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


    extern const std::unordered_map<std::string, std::vector<std::string> (*)(std::vector<std::string>, User&)> commands{
        {std::string("USER"), USER},
        {std::string("PASS"), UNIMPLEMENTED_ERROR},
        {std::string("ACCT"), UNIMPLEMENTED_ERROR},
        {std::string("CWD"), UNIMPLEMENTED_ERROR},
        {std::string("CDUP"), UNIMPLEMENTED_ERROR},
        {std::string("SMNT"), UNIMPLEMENTED_ERROR},
        {std::string("REIN"), UNIMPLEMENTED_ERROR},
        {std::string("QUIT"), UNIMPLEMENTED_ERROR},
        {std::string("PORT"), PORT},
        {std::string("PASV"), UNIMPLEMENTED_ERROR},
        {std::string("MODE"), UNIMPLEMENTED_ERROR},
        {std::string("TYPE"), UNIMPLEMENTED_ERROR},
        {std::string("STRU"), UNIMPLEMENTED_ERROR},
        {std::string("ALLO"), UNIMPLEMENTED_ERROR},
        {std::string("REST"), UNIMPLEMENTED_ERROR},
        {std::string("STOR"), UNIMPLEMENTED_ERROR},
        {std::string("STOU"), UNIMPLEMENTED_ERROR},
        {std::string("RETR"), RETR},
        {std::string("LIST"), UNIMPLEMENTED_ERROR},
        {std::string("NLST"), UNIMPLEMENTED_ERROR},
        {std::string("APPE"), UNIMPLEMENTED_ERROR},
        {std::string("RNFR"), UNIMPLEMENTED_ERROR},
        {std::string("DELE"), UNIMPLEMENTED_ERROR},
        {std::string("RMD"), UNIMPLEMENTED_ERROR},
        {std::string("MKD"), UNIMPLEMENTED_ERROR},
        {std::string("PWD"), UNIMPLEMENTED_ERROR},
        {std::string("ABOR"), UNIMPLEMENTED_ERROR},
        {std::string("SYST"), SYST},
        {std::string("STAT"), UNIMPLEMENTED_ERROR},
        {std::string("HELP"), UNIMPLEMENTED_ERROR},
        {std::string("SITE"), UNIMPLEMENTED_ERROR},
        {std::string("NOOP"), UNIMPLEMENTED_ERROR} 
    };

    std::vector<std::string> UNIMPLEMENTED_ERROR(std::vector<std::string>, User& user) {
        user.sendResponse(502, "Not implemented.");
        std::vector<std::string> empty;
        return empty;
    }    

    std::vector<std::string> USER(std::vector<std::string> args, User& u) {
        std::vector<std::string> empty;
        if (args.size() != 2) {
            u.sendResponse(501, "Invalid number of arguments");
            return empty;
        }
        u.setName(args[1]);
        u.sendResponse(230, "Logged in, continue");
        return empty;
    }

    std::vector<std::string> SYST(std::vector<std::string> args, User& u) {
        std::vector<std::string> empty;
        if (args.size() != 1) {
            u.sendResponse(501, "Invalid number of arguments");
            return empty;
        }
        u.sendResponse(215, "UNIX"); //I couldn't figure out what to call to get this at runtime lol
        return empty;
    }

    std::vector<std::string> RETR(std::vector<std::string> args, User& u) {
        std::vector<std::string> response;
        //move this to DTP once uid enters the mix
        if (access(args[1].c_str(), R_OK) != 0) {
            return response;
        }
        response.push_back(args[1]);
        if (u.getPassive()) {
            response.push_back("P");
            response.push_back("D");
        }
        else {
            response.push_back("A");
            response.push_back("D");
            struct sockaddr_in addr;
            socklen_t addr_size = sizeof(struct sockaddr_in);
            getpeername(u.getDescriptor(), (struct sockaddr *)&addr, &addr_size);
            std::string address(inet_ntoa(addr.sin_addr));
            response.push_back(address);
            response.push_back(std::to_string(u.getPort()));
        }
        return response;
    }

    std::vector<std::string> PORT(std::vector<std::string> args, User& u) {
        std::vector<std::string> empty;
        if (args.size() != 2) {
            u.sendResponse(501, "Invalid number of arguments.");
        }



        u.sendResponse(200, "OKAY");
        return empty;
    }
}