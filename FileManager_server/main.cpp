//
// Created by unl on 02.04.17.
//
#include <iostream>
#include <fstream>
#include <cstring>
#include "BasicConnection.h"

vector<byte>& operator+=(vector<byte>& vec, const string str){
    vec.resize( vec.size() + str.length() );

    if (str.length() > 0)
        bcopy( &str[0], &vec[ vec.size() - str.length() ] , str.length() );

    return vec;
}

string vecToString(vector<byte> vec){
    string res(vec.size(),'v');
    if (vec.size() > 0)
        bcopy(  &vec[0], &res[0], vec.size() );
    return res;
}

class Server : public BasicConnection
{
public:

    Server(int32_t inPort) {
        port = inPort;
    }

    void run(){
        int32_t n, selfsockfd;
        sockaddr_in cli_addr;
        selfsockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (selfsockfd < 0)
            throw Server::errors::openingSocket;

        bzero((char *) &address, sizeof(address));

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if ( bind( selfsockfd, (sockaddr*) &address, sizeof(address) ) < 0 )
        throw  Server::errors::binding;

        listen(selfsockfd,10);

        socklen_t clilen = sizeof(cli_addr);

        while(1){
            sockfd = accept(selfsockfd,(struct sockaddr *) &cli_addr, &clilen);

            vector<byte> res = recieveRawBytes();

            for (auto &i : res)
                std::cout<< i;
            std::cout << std::endl;

            const byte request = res[0];

            std::fstream dbUsers("dbUsers");

            sendRawBytes(res);
            switch  (request){
                case (byte)Server::requests::REGISTRATION :

                    res.erase(res.begin());

                    dbUsers.seekg(dbUsers.end);

                    dbUsers << vecToString(res);
                    dbUsers.close();
                    break;
                case (byte)Server::requests::LOGIN :

                    break;


            }
        }
    }
};

int main() {

    try {
        Server serv(2292);

        serv.run();

    }
    catch ( Server::errors & er){
        std::cout<<"Error code "<<(int)er;
        return -1;
    }
}