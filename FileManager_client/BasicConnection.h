//
// Created by unl on 06.04.17.
//
#pragma once

#include <netdb.h>
#include <unistd.h>
#include <string>
#include <vector>

typedef uint8_t byte;

class Connection {
public:
    int32_t  Sockfd, Port;
    std::string Hostname;
    sockaddr_in Address;

    enum class errors{
        NoErrors,
        OpeningSocket,
        Binding,
        Accepting,
        Reading,
        Responding,
        SmallInput,
        Writing,
        ImplementedWork,
        FindingHost,
        Connecting,
        UnknownRequest,
        AlreadyRegistered,
        BadLogin,
        EmptyResponce,
        HackingAttempt,
        BadFile
    };

    enum class requests{
        REGISTRATION = 'R',
        LOGIN = 'L',
        FILE_LISTING = 'F',
        UPLOAD = 'U',
        DOWNLOAD = 'D'
    };

    virtual void Run();
    void SendRawBytes(const std::vector<byte>& Info);
    std::vector<byte> RecieveRawBytes();
};