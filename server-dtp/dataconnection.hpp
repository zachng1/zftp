#ifndef __DATACON_H
#define __DATACON_H
//represents a single data connection
//subclass for download vs upload

namespace zftp{
class DataConnection {
    public:
    virtual int transferFile(int bytes) = 0;
};
}

#endif