#include "user.h"

namespace zftp {
    User::User(int fd) : fd{fd}
    {

    }

    int User::getDescriptor() {
        return fd;
    }

    std::string User::readCommand() {
        char buf[255];
        std::string command;
        while (read(fd, buf, 255) != 0) {
            command += std::string(buf);
        }
        return command;
    }

    int User::sendResponse(uint code, std::string message) {
        ssize_t bytesWritten;
        if (code > 999) return -1;
        ssize_t len = message.length() + 6; //3 for the code, 1 for the space, 2 for CRLF
        char * buf = (char *) malloc(len * sizeof(char));
        snprintf(buf, len, "%03d %s\r\n", code, message.c_str());
        while ((bytesWritten = write(fd, buf, len)) != 0) {
            if (bytesWritten < 0) return -1;
        }
        return 0;
    }

    int User::sendMultilineResponse(uint code, std::vector<std::string> messages) {
        ssize_t bytesWritten;
        if (code > 999) return -1;
        ssize_t len = 6; //3 for the code, 1 for the dash, 2 for crlf
        for (auto i: messages) {
            len += i.length()+1;//one extra for each newline 
        }
        char * buf = (char *) malloc(len * sizeof(char));
        snprintf(buf, 5, "%03d-", code);
        buf += 4;
        for (auto i: messages) {
            snprintf(buf, i.length()+2, "%s\n", i.c_str()); // +2 for the null term
            buf += i.length() + 1; //overwrite null term
        }
        buf -= 1; //overwrite last '/n'
        snprintf(buf, 2, "\r\n");
        while ((bytesWritten = write(fd, buf, len)) != 0) {
            if (bytesWritten < 0) return -1;
        }
        return 0;
    }
}