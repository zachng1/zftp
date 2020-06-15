#ifndef __USER_H
#define __USER_H

//this object represents a single User PI connection

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>


//C Socket headers:
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <unistd.h>
#include <errno.h>

namespace zftp{
    class User {
        public:
        User(int fd);
        int getDescriptor();
        std::string getName();
        void setName(std::string name);
        int sendResponse(uint code, std::string message="");
        int sendMultilineResponse(uint code, std::vector<std::string> messages);
        std::vector<std::string> readCommand();

        private:
        int fd;
        std::string username;

    };
}

#endif