//
// Created by unl on 02.04.17.
//
#pragma once

#include <string.h>

#include "BasicConnection.h"

std::vector<byte>& operator+=(std::vector<byte>& vec, const std::string Str);
std::string VecToString(std::vector<byte> Vec);
int64_t VecToNum(std::vector<byte> Vec);

class Client : public Connection {
public:
    std::string Username;
    std::string Password;
    Client(uint32_t InPort , std::string InHostname);
    void Run() override;
    void Registrate(std::string Name, std::string pass);
    void Login(std::string Name, std::string pass);
    std::vector<std::string> GetFileListing(std::string path);
    void DownloadFile(std::string path , std::string& filename);
    void UploadFile(std::string Path, std::string Filename);
};