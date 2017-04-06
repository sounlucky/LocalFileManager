//
// Created by unl on 02.04.17.
//
#include <cstring>
#include "BasicConnection.h"

class Server : public BasicConnection
{
public:

    Server(int32_t inPort) {
        port = inPort;
    }

    void run() override{
        int32_t n, selfsockfd;
        sockaddr_in cli_addr;
        selfsockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (selfsockfd < 0)
            throw Server::errors::ERonOpeningSocket;

        bzero((char *) &address, sizeof(address));

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if ( bind( selfsockfd, (sockaddr*) &address, sizeof(address) ) < 0 )
        throw  Server::errors::ERonBinding;

        listen(selfsockfd,10);

        socklen_t clilen = sizeof(cli_addr);
    }

};

int main() {

    try {
        Server serv(22901);
        serv.run();
    }
    catch ( Server::errors & er){
        std::cout<<"Error code "<<(int)er;
        return -1;
    }
}