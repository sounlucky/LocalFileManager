//
// Created by unl on 02.04.17.
//
#include <iostream>
#include <fstream>
#include <iterator>
#include <sstream>
#include <array>
#include <memory>
#include "string.h"
#include "BasicConnection.h"

std::vector<byte> &operator+=(std::vector<byte> &vec, const std::string str) {
    vec.resize(vec.size() + str.length());
    if (str.length() > 0)
        bcopy(&str[0], &vec[vec.size() - str.length()], str.length());
    return vec;
}

std::string vecToString(std::vector<byte> vec) {
    std::string res(vec.size(), 'v');
    if (vec.size() > 0)
        bcopy(&vec[0], &res[0], vec.size());
    return res;
}

class Server : public BasicConnection {

public:

    Server(uint32_t inPort) {
        port = inPort;
    }

    const std::string STORAGE_FOLDER_NAME = "fileStorage/";

    void run() {
        int32_t n, selfsockfd;
        sockaddr_in cli_addr;
        selfsockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (selfsockfd < 0)
            throw errors::openingSocket;
        bzero((char *) &address, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(static_cast<uint16_t>(port));
        if (bind(selfsockfd, (sockaddr *) &address, sizeof(address)) < 0)
            throw errors::binding;
        listen(selfsockfd, 10);
        socklen_t clilen = sizeof(cli_addr);
        while (1) {
            std::cout << "\n waiting... \n";
            sockfd = accept(selfsockfd, (struct sockaddr *) &cli_addr, &clilen);
            std::vector<byte> request = recieveRawBytes(),
                    respond;
            //debugging purposes , rm after
            for (auto i : request)
                std::cout << i;
            std::cout << std::endl;
            const byte reqLetter = request[0];
            std::fstream dbUsers("dbUsers");
            dbUsers.seekg(0, std::ios::end);
            int64_t dbUsersSize = dbUsers.tellg();
            std::string dbUsersInformation;
            if (dbUsersSize <= 0){
                std::ofstream fileStream("dbUsers" ,std::ios::trunc);
                fileStream.close();
                dbUsersInformation = "";
            }
            else {
                dbUsers.seekg(0, std::ios::beg);
                std::string temp(static_cast<uint64_t>(dbUsersSize), '\0');
                dbUsersInformation = temp;
                dbUsers.read(&dbUsersInformation.at(0), dbUsersSize);
            }
            //ATTENTION! Its closed here!
            dbUsers.close();
            switch (static_cast<requests>(reqLetter)) {
                case requests::REGISTRATION :
                    //request example: R<username>&<psswrd>
                    if (doesUserExist(request, dbUsersInformation)) {
                        respond += std::to_string((uint8_t) errors::alreadyRegistered);
                    } else {
                        registrate(request, dbUsers);
                        respond += std::to_string((uint8_t) errors::noErrors);
                    }
                    break;
                case requests::LOGIN:
                    //request example: L<username>&<password>
                    if (doesAccExist(request, dbUsersInformation))
                        respond += std::to_string((uint8_t) errors::noErrors);
                    else
                        respond += std::to_string((uint8_t) errors::badLogin);
                    //responce is obv 1 byte
                    break;
                case requests::FILE_LISTING:
                    //request example: F<username>&<password>&<folder1/folder2/>
                    respond += sendFileListing(request, dbUsersInformation);
                    //responce is obv 1 byte
                    break;
                case requests::UPLOAD:
                    //request example: U<username>&<password>&<folder1/folder2/file>&<bytes>
                    writeFile(request , dbUsersInformation);
                    respond += std::to_string((uint8_t) errors::noErrors);
                    break;
                case requests::DOWNLOAD:
                    //request example: D<username>&<password>&<folder1/folder2/file>
                    respond = getFile(request , dbUsersInformation);
                    break;
            }
            sendRawBytes(respond);
        }
    }

    std::vector<byte> getFile(std::vector<byte> request , std::string& dbUsersInformation) {
        std::string path = parse(request , dbUsersInformation);
        std::ifstream fileStream(STORAGE_FOLDER_NAME + path , std::ios::binary | std::ios::ate);
        std::ifstream::pos_type fSize = fileStream.tellg();
        if (fSize <= 0)
            throw errors::hackingAttempt;
        std::vector<byte> res(static_cast<uint64_t>(fSize));
        fileStream.seekg(0 , std::ios::beg);
        fileStream.read(reinterpret_cast<char*>(&res.at(0)) , fSize);
        fileStream.close();
        return res;
    }

    void writeFile(std::vector<byte> request , std::string& dbUsersInformation){
        std::string pathAndBytes =   parse(request , dbUsersInformation),
                                path;
        while (pathAndBytes.front() != '&' && pathAndBytes.length() > 0){
            path += pathAndBytes.front();
            pathAndBytes = pathAndBytes.substr(1 , pathAndBytes.length());
        }
        std::ofstream fileStream(STORAGE_FOLDER_NAME + path , std::ios::binary | std::ios::trunc);
        fileStream.write(reinterpret_cast<const char*>(&request[pathAndBytes.length() + 1]) , request.size() - pathAndBytes.length() - 1);
        fileStream.close();
    }

    std::string exec(const char *cmd) {
        constexpr int16_t BUFFER_LENGTH = 0xFF;
        std::array<char, BUFFER_LENGTH> buffer;
        std::string res;
        std::shared_ptr<FILE> cmdReader(popen(cmd, "r"), pclose);
        while (!feof(cmdReader.get())) {
            if (fgets(buffer.data(), BUFFER_LENGTH, cmdReader.get()) != NULL) {
                res += buffer.data();
            }
        }
        return res;
    }

    std::string parse(std::vector<byte> request, std::string& dbUsersInformation){
        // function converts
        // Auser&pass&text -> text
        // and also checking accExistence
        std::vector<byte> userData;   // user&pass
        std::string rawData;          // text
        std::vector<std::string>::size_type i = 0;
        while (request[i] != '&') {
            userData.push_back(request[i]);
            i++;
        }
        //found user
        userData.push_back('&');
        i++;
        while (request[i] != '&') {
            userData.push_back(request[i]);
            i++;
        }
        //found pass
        i++;//skip '&'
        while (i < request.size()) {
            rawData += request[i];
            i++;
        }
        if (!doesAccExist(userData, dbUsersInformation))
            throw std::to_string((uint8_t) errors::badLogin);
        return rawData;
    }

    std::string sendFileListing(std::vector<byte> &request, std::string &dbUsersInformation) {
        std::string path = parse(request , dbUsersInformation),
            folders = exec(std::string("cd " + STORAGE_FOLDER_NAME + path + "; ls -d */").c_str()),
                files = exec(std::string("cd " + STORAGE_FOLDER_NAME + path + "; ls -p | grep -v /").c_str());
        // dont crash please
        if (folders == "ls: cannot access '*/': No such file or directory")
            return files;
        else
            return folders + files;
}

    void registrate(const std::vector<byte> &request, std::fstream &dbUsers) {
        if (request.size() < 12)//too small
            throw errors::hackingAttempt;
        dbUsers.open("dbUsers", std::ios::app);
        dbUsers.write(reinterpret_cast<const char*>(&request[1]), request.size() - 1);
        dbUsers << std::endl;
        dbUsers.close();
    }

    bool doesAccExist(const std::vector<byte> &request, const std::string &fileInfo) {
        std::string line;
        std::string::size_type i = 0;
        while (i < fileInfo.length()) {
            switch (fileInfo[i]) {
                case '\n':
                    //obv beginning of the new line
                    if (line == vecToString(request).substr(1, request.size())) {
                        std::cout << "account does exist" << std::endl;
                        return true;
                    }
                    line = "";
                    break;
                default:
                    line += fileInfo[i];
                    break;
            }
            i++;
        }
        std::cout << "account doesnt exist" << std::endl;
        return false;
    }

    bool doesUserExist(const std::vector<byte> &request, const std::string &fileInfo) {
        std::string uName;
        //figure out username
        for (auto i = request.begin() + 1;
             i != request.end(); i++) {//we're starting with 2nd symbol coz 1st is meaningLetter
            switch (*i) {
                case '>':
                case '~':
                    //ignore those
                    break;
                case '&':
                    //ending 'for'
                    //because we dont need password
                    i = request.end() - 1;
                    break;
                default:
                    uName += *i;
                    break;
            }
        }
        std::string currUsername;
        std::string::size_type i = 0;
        while (i < fileInfo.length()) {
            switch (fileInfo[i]) {
                case '&':
                    //check if he has found one
                    if (currUsername == uName) {
                        std::cout << " user does exist" << std::endl;
                        return true;
                    }
                    //dont really need passwords, just skip 'em
                    while (fileInfo[++i] != '\n')
                        if (i < fileInfo.length())
                            break;
                    break;
                case '\n':
                    //obv its beginning of the next username
                    currUsername = "";
                    break;
                default:
                    currUsername += fileInfo[i];
                    break;
            }
            i++;
        }
        std::cout << "user does not exist" << std::endl;
        return false;
    }
};

int main() {
    try {
        constexpr int32_t PORT = 14941;
        Server serv(PORT);
        serv.run();
    }
    catch (Server::errors &er) {
        std::cout << "Server shut down. Error code " << (int16_t) er;
        return (int) er;
    }
}