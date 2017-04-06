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
    vector<string> logs;
    int32_t  sockfd, port;
    string hostname;
    sockaddr_in address;

    BasicConnection()  = default;
    ~BasicConnection() = default;

    virtual void run(){
        throw errors::ERnotImplementedWork;
    };

    enum class errors{
        ERonOpeningSocket,
        ERonBinding,
        ERonAccepting,
        ERonReading,
        ERonResponding,
        ERtooSmallInput,
        ERonWriting,
        ERnotImplementedWork,
        ERonFindingHost,
        ERonConnecting
    };

    void sendRawBytes(const vector<byte>& info){

        uint64_t lengthBuffer = info.size();
        if (write(sockfd, &lengthBuffer, sizeof(lengthBuffer)) < 0)
            throw errors ::ERonWriting;

        if ( write(sockfd, &info.at(0), info.size() ) < 0)
            throw errors ::ERonWriting;

        return;
    }

    vector<byte> recieveRawBytes(){

        uint64_t lengthBuffer = 0;
        if (read(sockfd, &lengthBuffer, sizeof(lengthBuffer)) < 0)
            throw errors::ERonReading;

        vector<byte> responceBytes(lengthBuffer);
        read(sockfd , &responceBytes.at(0) , lengthBuffer);

        return responceBytes;
    }
};


#endif //FILEMANAGER_SERVER_BASICCONNECTION_H
