#include <iostream>

//C Socket headers:
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <unistd.h>



int main(int argc, char ** argv) {
    int writeToPi = std::stoi(std::string(argv[1]));
    int readFromPi = std::stoi(std::string(argv[2]));
    
    //signal to PI that we are ready to start sending and recieving data
    char readyBuf[1]{'A'};
    if (write(writeToPi, readyBuf, 1) < 0) {
        std::cout << "Error writing to PI" << std::endl;
        return 1;
    };
    while (1);
    return 0;
}