#include "handlerfunctions.h"

namespace zftp {
    // These are a number of helper functions for the main init handling method
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
    }

    void initHandling(bool& error, std::unordered_map<int, User>& users, std::shared_timed_mutex& usersMutex, int selfPipeServerAlert, int writeToDTP, int readFromDTP) {
        int readyCount;
        std::vector<struct pollfd> pollingVector;
        std::vector<struct pollfd> newFds;
        std::vector<struct pollfd> hangups;
        std::string newfdstring;

        pollingVector = createPollVector(users, selfPipeServerAlert, usersMutex);  

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
                else if (pollfd.revents & POLLIN) {
                    std::vector<std::string> message;
                    std::shared_lock<std::shared_timed_mutex> lock(usersMutex);
                    User& user = users.at(pollfd.fd);
                    message = user.readMessage();
                    if (message[0] == "DISCONNECTED" || message.empty()) {
                        hangups.push_back(pollfd);
                    }
                    else {
                        //accesses the global commands dict
                        //using the received command string as 
                        //then calls that function with the current user
                        try {
                            std::cout << user.getName() << ": " << message[0] << std::endl;
                            zftp::commands.at(message[0])(message, user);
                        }
                        catch (std::out_of_range &e) {
                            user.sendResponse(500, "Command not recognised");
                        }
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
    void UNIMPLEMENTED_ERROR(std::vector<std::string>, User& user) {
        user.sendResponse(502, "Not implemented.");
    }


    extern const std::unordered_map<std::string, void (*)(std::vector<std::string>, User&)> commands{
        {std::string("USER"), USER},
        {std::string("PASS"), UNIMPLEMENTED_ERROR},
        {std::string("ACCT"), UNIMPLEMENTED_ERROR},
        {std::string("CWD"), UNIMPLEMENTED_ERROR},
        {std::string("CDUP"), UNIMPLEMENTED_ERROR},
        {std::string("SMNT"), UNIMPLEMENTED_ERROR},
        {std::string("REIN"), UNIMPLEMENTED_ERROR},
        {std::string("QUIT"), UNIMPLEMENTED_ERROR},
        {std::string("PORT"), UNIMPLEMENTED_ERROR},
        {std::string("PASV"), UNIMPLEMENTED_ERROR},
        {std::string("MODE"), UNIMPLEMENTED_ERROR},
        {std::string("TYPE"), UNIMPLEMENTED_ERROR},
        {std::string("STRU"), UNIMPLEMENTED_ERROR},
        {std::string("ALLO"), UNIMPLEMENTED_ERROR},
        {std::string("REST"), UNIMPLEMENTED_ERROR},
        {std::string("STOR"), UNIMPLEMENTED_ERROR},
        {std::string("STOU"), UNIMPLEMENTED_ERROR},
        {std::string("RETR"), UNIMPLEMENTED_ERROR},
        {std::string("LIST"), UNIMPLEMENTED_ERROR},
        {std::string("NLST"), UNIMPLEMENTED_ERROR},
        {std::string("APPE"), UNIMPLEMENTED_ERROR},
        {std::string("RNFR"), UNIMPLEMENTED_ERROR},
        {std::string("DELE"), UNIMPLEMENTED_ERROR},
        {std::string("RMD"), UNIMPLEMENTED_ERROR},
        {std::string("MKD"), UNIMPLEMENTED_ERROR},
        {std::string("PWD"), UNIMPLEMENTED_ERROR},
        {std::string("ABOR"), UNIMPLEMENTED_ERROR},
        {std::string("SYST"), UNIMPLEMENTED_ERROR},
        {std::string("STAT"), UNIMPLEMENTED_ERROR},
        {std::string("HELP"), UNIMPLEMENTED_ERROR},
        {std::string("SITE"), UNIMPLEMENTED_ERROR},
        {std::string("NOOP"), UNIMPLEMENTED_ERROR} 
    };

    void USER(std::vector<std::string> args, User& u) {
        if (args.size() != 2) {
            u.sendResponse(501, "Invalid number of arguments");
            return;
        }
        u.setName(args[1]);
        u.sendResponse(230, "Logged in, continue");
    }

    void SYST(std::vector<std::string> args, User& u) {
        if (args.size() != 1) {
            u.sendResponse(501, "Invalid number of arguments");
            return;
        }
        u.sendResponse(215, "UNIX"); //I couldn't figure out what to call to get this at runtime lol
    }
}