#include "dataconnection.hpp"

namespace zftp{
DownloadConnection::DownloadConnection(int fd, std::string path) {
    
}
int DownloadConnection::transferFile(int bytes) {

}

UploadConnection::UploadConnection(int fd, std::string path) {
    
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
            if (errno = EINPROGRESS || errno == EAGAIN) {
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