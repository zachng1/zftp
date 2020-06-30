#ifndef __USER_H
#define __USER_H

//this object represents a single User PI connection

#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>


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
        User(int fd, int port = 21);
        int getDescriptor();
        std::string getName();
        void setName(std::string name);
        std::string getLastCommand();
        void setLastCommand(std::string command);


        int sendResponse(uint code, std::string message="");
        int sendMultilineResponse(uint code, std::vector<std::string> messages);
        //Returns a vector containing only "DISCONNECTED" on disconnect
        //or an empty vector on some other error.
        std::vector<std::string> readMessage();

        private:
        int fd;
        std::string username;
        std::string lastCommand; // AKA the command currently being processed
        int port; //for passive connections

    };
}

#endif