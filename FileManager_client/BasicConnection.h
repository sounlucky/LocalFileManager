//
// Created by unl on 06.04.17.
//
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <vector>

using std::string; using std::vector;


#ifndef FILEMANAGER_SERVER_BASICCONNECTION_H
#define FILEMANAGER_SERVER_BASICCONNECTION_H

typedef uint8_t byte;

class BasicConnection {

public:
    int32_t  sockfd, port;
    string hostname;
    sockaddr_in address;

    BasicConnection()  = default;
    ~BasicConnection() = default;

    virtual void run(){
        throw errors::implementedWork;
    };

    enum class errors{
        noErrors,
        openingSocket,
        binding,
        accepting,
        reading,
        responding,
        smallInput,
        writing,
        implementedWork,
        findingHost,
        connecting,//10
        unknownRequest,
        alreadyRegistered,
        badLogin,
        emptyResponce
    };

    enum class requests{
        REGISTRATION = 'R',
        LOGIN = 'L',
        FILE_LISTING = 'F',
        UPLOAD = 'U',
        DOWNLOAD = 'D'
    };

    void sendRawBytes(const vector<byte>& info){
        uint64_t lengthBuffer = info.size();
        if (write(sockfd, &lengthBuffer, sizeof(lengthBuffer)) < 0)
            throw errors ::writing;
        if (lengthBuffer > 0)
            if ( write(sockfd, &info.at(0), info.size() ) < 0)
                throw errors ::writing;
        return;
    }

    vector<byte> recieveRawBytes(){
        uint64_t lengthBuffer = 0;
        if (read(sockfd, &lengthBuffer, sizeof(lengthBuffer)) < 0)
            throw errors::reading;
        vector<byte> responceBytes(lengthBuffer);
        if (lengthBuffer > 0)
            read(sockfd , &responceBytes.at(0) , lengthBuffer);
        return responceBytes;
    }

    vector<string> requestFiles(string path){

    }
};


#endif //FILEMANAGER_SERVER_BASICCONNECTION_H
