#ifndef __DATACON_H
#define __DATACON_H

//represents a single data connection
//subclass for download vs upload

//C Socket headers:
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

class DataConnection {
    public:
    DataConnection(int fileFd);
    


    private:
    int sockFd;
    int fileFd;

};

#endif