#include "BasicConnection.h"

void Connection::Run() {
    throw errors::ImplementedWork;
}

void Connection::SendRawBytes(const std::vector<byte> &info) {
    uint64_t LengthBuffer = info.size();
    if (write(Sockfd, &LengthBuffer, sizeof(LengthBuffer)) < 0)
        throw errors ::Writing;
    if (LengthBuffer > 0)
        if ( write(Sockfd, &info.at(0), info.size() ) < 0)
            throw errors ::Writing;
    return;
}

std::vector<byte> Connection::RecieveRawBytes() {
    uint64_t LengthBuffer = 0;
    if (read(Sockfd, &LengthBuffer, sizeof(LengthBuffer)) < 0)
        throw errors::Reading;
    std::vector<byte> ResponceBytes(LengthBuffer);
    if (LengthBuffer > 0)
        read(Sockfd , &ResponceBytes.at(0) , LengthBuffer);
    return ResponceBytes;
}
