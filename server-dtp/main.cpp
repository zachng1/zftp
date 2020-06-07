#include <iostream>
#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#endif
#ifdef _WIN32
#include <WinSock2.h>
#endif


int main() {
    
}