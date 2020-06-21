#ifndef __DATACON_H
#define __DATACON_H
//represents a single data connection
//subclass for download vs upload

namespace zftp{
class DataConnection {
    public:
    virtual DataConnection(int fileFd, int sockFd) = 0;
    virtual int transferFile(int bytes) = 0;
    
    private:
    int sockFd;
    int fileFd;

};
}