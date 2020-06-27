#ifndef __DATACON_H
#define __DATACON_H
//represents a single data connection
//subclass for download vs upload
#include <string>
#include <vector>
#include <iostream>

//C Socket headers:
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <fcntl.h>

namespace zftp{
class DataConnection {
    public:
    virtual int transferFile(int bytes) = 0;
};

class UploadConnection : public DataConnection {
    public:
    UploadConnection(){};
    UploadConnection(int fd, std::string path);
    int transferFile(int bytes);

    private:
    int fd;
    int fileSocket;
};

class DownloadConnection : public DataConnection {
    public:
    DownloadConnection(){};
    DownloadConnection(int fd, std::string path);
    int transferFile(int bytes);

    private:
    int fd;
    int fileSocket;
};

//Returns a new socket. It may not be connected yet. Check for
//writability and socket errors to make sure it is connected.
int getActiveConnectionFd(std::vector<std::string> command);

//Returns a new *listening* socket -- make sure it gets closed once
//the client makes a connection
int getPassiveConnectionFd(std::vector<std::string> command);
}

#endif