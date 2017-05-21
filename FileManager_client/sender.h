//
// Created by unl on 02.04.17.
//
#pragma once

#include <string.h>

#include "BasicConnection.h"

std::vector<byte>& operator+=(std::vector<byte>&, const std::string);
std::string VecToString(std::vector<byte>);
int64_t VecToNum(std::vector<byte>);

class Client : public Connection {
public:
    std::string Username;
    std::string Password;
    Client(uint32_t, std::string);
    void Run() override;
    void Registrate(std::string,std::string);
    void Login(std::string, std::string);
    std::vector<std::string> GetFileListing(std::string);
    void DownloadFile(std::string, std::string&);
    void UploadFile(std::string, std::string);
};