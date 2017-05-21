//
// Created by unl on 02.04.17.
//
#include "Server.h"

int main() {
    try {
        constexpr int32_t PORT = 14841;
        Server Serv(PORT);
        Serv.Run();
    }
    catch (Server::errors &er) {
        std::cout << "Server shut down. Error code " << (int16_t) er;
        return (int) er;
    }
}