#include <iostream>
#include <thread>
#include <chrono>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

void setNonBlock(int pipe);

int main() {   
    int WritePipe[2], ReadPipe[2];
    pid_t DTPProcId, PIProcId;


    if (pipe(WritePipe) < 0 || pipe(ReadPipe) < 0) {
        std::cerr << "Could not initiate pipes." << std::endl;
        return 1;
    }
    
    int writeToPI = ReadPipe[1];
    int writeToDTP = WritePipe[1];
    int readFromPI = WritePipe[0];
    int readFromDTP = ReadPipe[0];
    setNonBlock(writeToPI);
    setNonBlock(writeToDTP);
    setNonBlock(readFromPI);
    setNonBlock(readFromDTP);

    if ((DTPProcId = fork()) == 0) {
        close(writeToDTP);
        close(readFromDTP);
        char * argv[4];
        char writeBuf[10], readBuf[10];
        snprintf(writeBuf, 10, "%d", writeToPI);
        snprintf(readBuf, 10, "%d", readFromPI);
        argv[0] = (char*) "server-dtp/dtp";
        argv[1] = writeBuf;
        argv[2] = readBuf;
        argv[3] = (char *) nullptr;
        execv(argv[0], argv);
        std::cerr << "Failed to start DTP." << std::endl;
        return 1;

    }
    else if (DTPProcId == -1) {
        std::cerr << "Failed to start DTP." << std::endl;
        return 1;
    }
    close(readFromPI);
    close(writeToPI);

    if ((PIProcId = fork()) == 0) {
        char * argv[4];
        char writeBuf[10], readBuf[10];
        snprintf(writeBuf, 10, "%d", writeToDTP);
        snprintf(readBuf, 10, "%d", readFromDTP);
        argv[0] = (char*) "server-pi/pi";
        argv[1] = writeBuf;
        argv[2] = readBuf;
        argv[3] = (char *) nullptr;
        execv(argv[0], argv);
        std::cerr << "Failed to start PI." << std::endl;
        return 1;
    }
    else if (PIProcId == -1) {
        std::cerr << "Failed to start PI." << std::endl;
    }
    close(readFromDTP);
    close(writeToDTP);

    //wait for either child to return
    //when one does, kill the other.
    //(Need to implement a more graceful exit -- i.e. ensure that DTP can complete any transfers)
    //But this is mainly for bugs    
    int status;
    int returnedPID = wait(&status);
    if (returnedPID == PIProcId) {
        std::cerr << "PI Crash" << std::endl;
        kill(DTPProcId, SIGTERM);
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        if (waitpid(DTPProcId, nullptr, WNOHANG) != DTPProcId) {
            kill(DTPProcId, SIGKILL);
            waitpid(DTPProcId, nullptr, 0);
        }
    }
    else {
        std::cerr << "DTP Crash" << std::endl;
        kill(PIProcId, SIGTERM);
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        if (waitpid(PIProcId, nullptr, WNOHANG) != PIProcId) {
            kill(PIProcId, SIGKILL);
            waitpid(PIProcId, nullptr, 0);
        }
    }
        
    return 0;
}

void setNonBlock(int pipe) {
    int flags = fcntl(pipe, F_GETFL, 0);
    fcntl(pipe, F_SETFL, flags | O_NONBLOCK);
}