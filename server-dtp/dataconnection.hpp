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
#include <unistd.h>

namespace zftp{
//the following 3 classes are named from the perspective of the 
//USER -- download connection means the server is *sending* data etc.

class DataConnection {
    public:
    virtual int transferFile(int bytes){};
    virtual bool resetDescriptor(){};
};

class UploadConnection : public DataConnection {
    public:
    UploadConnection(int fd, std::string path);
    int transferFile(int bytes);
    bool resetDescriptor();

    private:
    int fd;
    int fileSocket;
};

class DownloadConnection : public DataConnection {
    public:
    DownloadConnection(int fd, std::string path);
    int transferFile(int bytes);
    bool resetDescriptor();

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