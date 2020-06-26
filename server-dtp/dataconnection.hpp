#ifndef __DATACON_H
#define __DATACON_H
//represents a single data connection
//subclass for download vs upload
#include <string>
#include <vector>

namespace zftp{
class DataConnection {
    public:
    virtual int transferFile(int bytes) = 0;
};

class UploadConnection : public DataConnection {
    public:
    UploadConnection(int fd, int fileFD);
    int transferFile(int bytes);

    private:
    int fd;
    int fileSocket;
};

class DownloadConnection : public DataConnection {
    public:
    DownloadConnection(int fd, int fileFD);
    int transferFile(int bytes);

    private:
    int fd;
    int fileSocket;
};

//Returns a new socket.
int newActiveConnection(std::vector<std::string> command);

//Returns a new *listening* socket -- make sure it gets closed once
//the client makes a connection
int newPassiveConnection(std::vector<std::string> command);
}

#endif