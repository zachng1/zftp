#include "user.h"

namespace zftp {
    User::User(int fd) : 
    fd{fd},
    lastCommand{""}
    {

    }

    int User::getDescriptor() {
        return fd;
    }

    std::string User::getName(){
        return username;
    }

    void User::setName(std::string name) {
        username = name;
    }

    std::vector<std::string> User::readMessage() {
        char buf[255];
        int bytesRead, end;
        std::string message;
        std::vector<std::string> args;
        errno = 0;
        while (bytesRead = read(fd, buf, 255)) {
            if (bytesRead < 0 && errno == EAGAIN) {
                break;
            }
            else if (bytesRead < 0) {
                return args;
            }
            message += std::string(buf);
        }

        if (bytesRead == 0) {
            args.push_back("DISCONNECTED");
        }
        else {
            end = message.find("\r\n", 0);
            if (end > 1) message = message.substr(0, end);
            std::istringstream stream{message};
            std::string arg;
            while (std::getline(stream, arg, ' ')) {
                args.push_back(arg);
            }
            //Command code can be in lower or uppercase
            //Convert to upper for accessing commands array
            std::transform(args[0].begin(), args[0].end(), args[0].begin(),
                [](unsigned char c){ return std::toupper(c); });
        }
        return args;

    }

    int User::sendResponse(uint code, std::string message) {
        if (code > 999) return -1;
        ssize_t len = message.length() + 7; //3 for the code, 1 for the space, 2 for CRLF, 1 for \0
        char * buf = (char *) malloc(len * sizeof(char));
        snprintf(buf, len, "%03d %s\r\n", code, message.c_str());
        if ((write(fd, buf, len)) < 0) return -1;
        return 0;
    }

    int User::sendMultilineResponse(uint code, std::vector<std::string> messages) {
        ssize_t bytesWritten;
        if (code > 999) return -1;
        ssize_t len = 7; //3 for the code, 1 for the dash, 2 for crlf, 1 for final nullterminator
        for (auto i: messages) {
            len += i.length()+1;//one extra for each newline 
        }
        char * buf = (char *) malloc(len * sizeof(char));
        snprintf(buf, 5, "%03d-", code);
        buf += 4;
        for (auto i: messages) {
            snprintf(buf, i.length()+2, "%s\n", i.c_str()); // +2 for the null terminator
            buf += i.length() + 1; //overwrite null term
        }
        buf -= 1; //overwrite last '/n'
        snprintf(buf, 3, "\r\n");
        while ((bytesWritten = write(fd, buf, len)) != 0) {
            if (bytesWritten < 0) return -1;
        }
        return 0;
    }
}