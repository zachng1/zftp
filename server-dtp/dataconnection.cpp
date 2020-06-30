#include "dataconnection.hpp"

namespace zftp{
DownloadConnection::DownloadConnection(int fd, std::string path) : fd{fd} {
    fileSocket = open(path.c_str(), O_RDONLY);
}
int DownloadConnection::transferFile(int bytes) {
    char * buf = (char *) malloc(sizeof(char) * bytes);
    char * bufstart = buf;
    int bytesSent, total = 0;
    //read exactly bytes into buf
    while (total < bytes) {
        if ((bytesSent = read(fileSocket, buf, bytes - total)) == 0) {
            break;
        }
        total += bytesSent;
        buf += bytesSent;
    }
    buf = bufstart;
    if (total == 0) {
        free(buf);
        return total;
    }
    int writeTotal = 0;
    //write exactly bytes to fd
    while (writeTotal < total) {
        errno = 0;
        if ((bytesSent = write(fd, buf, bytes - writeTotal)) < 0 && errno != EAGAIN) {
            buf = bufstart;
            free(buf);
            return bytesSent;
        }
        if (bytesSent == 0) {
            break;
        }
        writeTotal += bytesSent;
        buf += bytesSent;
    }
    buf = bufstart;
    free(buf);
    return writeTotal;
}

UploadConnection::UploadConnection(int fd, std::string path) {
    fileSocket = open(path.c_str(), O_WRONLY | O_CREAT);
}
int UploadConnection::transferFile(int bytes) {

}

int getActiveConnectionFd(std::vector<std::string> command) {
    struct addrinfo hints{}, *result;
    int resultFd = -1, flags;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(command[4].c_str(), command[5].c_str(), &hints, &result) < 0) return -1;
    for (struct addrinfo *i = result; i != nullptr; i = i->ai_next) {
        if ((resultFd = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) < 0) {
            continue;
        }
        errno = 0;
        if ((flags = fcntl(resultFd, F_GETFL, 0)) < 0) {
            resultFd = -1;
            continue;
        }
        if (fcntl(resultFd, F_SETFL, flags | O_NONBLOCK) <  0) {
            resultFd = -1;
            continue;
        }
        if (connect(resultFd, i->ai_addr, i->ai_addrlen) < 0) {
            if (errno == EINPROGRESS || errno == EAGAIN) {
                break;
            }
            resultFd = -1;
            continue;
        }
        break;
    }
    freeaddrinfo(result);
    return resultFd;
}

int getPassiveConnectionFd(std::vector<std::string> command) {
    std::cout << "Not implemented" << std::endl;
}
}