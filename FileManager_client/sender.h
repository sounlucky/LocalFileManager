//
// Created by unl on 02.04.17.
//
#include "BasicConnection.h"
#include <string.h>

std::vector<byte>& operator+=(std::vector<byte>& vec, const std::string str){
    vec.resize( vec.size() + str.length() );
    if (str.length() > 0)
        bcopy( &str[0], &vec[ vec.size() - str.length() ] , str.length() );
    return vec;
}

std::string vecToString(std::vector<byte> vec){
    std::string res(vec.size(),'\0');
    if (vec.size() > 0)
        bcopy(  &vec[0], &res[0], vec.size() );
    return res;
}

int64_t vecToNum(std::vector<byte> vec){
    std::string temp = vecToString(vec);
    return atoi( temp.c_str());//should test std::to_int64 !
}

class Client : public BasicConnection {
public:
    std::string username,
            password;
    Client(uint32_t inPort , std::string inHostname){
        port = inPort;
        hostname = inHostname;
    }

    void run() override {
        hostent *server;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
            throw errors::openingSocket;
        server = gethostbyname(hostname.c_str());
        if (server == NULL)
            throw Client::errors::findingHost;
        bzero((char *) &address, sizeof(address));
        address.sin_family = AF_INET;
        bcopy(server->h_addr, (char *) & address.sin_addr.s_addr, server->h_length);
        address.sin_port = htons(port);
        if ( connect( sockfd,(struct sockaddr *) &address, sizeof(address) ) < 0)
            throw Client::errors::connecting;
    }

    void registrate(std::string name, std::string pass){
        run();
        std::vector<byte> toSend;
        toSend += (char)Client::requests::REGISTRATION + name + '&' + pass;
        username = name; password = pass;
        sendRawBytes(toSend);
        std::vector<byte> respond = recieveRawBytes();
        close(sockfd);
        if (  vecToNum(respond) != (uint8_t) Client::errors::noErrors )
            throw (Client::errors)vecToNum(respond);
        return;
    }

    void login(std::string name, std::string pass){
        run();
        std::vector<byte> toSend;
        toSend += static_cast<char>(Client::requests::LOGIN) + name + '&' + pass;
        sendRawBytes(toSend);
        std::vector<byte> respond = recieveRawBytes();
        close(sockfd);
        if (  vecToNum(respond) != 0 )
            throw (Client::errors)vecToNum(respond);
        username = name;
        password = pass;
        return;
    }

    std::vector<std::string> getFileListing(std::string path){
        run();
        std::vector<byte> request;
        request += static_cast<char>(Client::requests::FILE_LISTING) + username + '&' + password + '&' + path;
        sendRawBytes(request);
        std::vector<byte> respond = recieveRawBytes();
        close(sockfd);
        std::vector<std::string> files;
        if (path != "")
            files.push_back("..");
        std::string temp = "";
        //parsing
        for (auto ch : respond){
            if (ch == '\n') {
                files.push_back(temp);
                temp = "";
            }
            else
                temp += ch;
        }
        return files;
    }

    void downloadFile(std::string path , std::string& filename){
        run();
        std::vector<byte> request;
        request += static_cast<char>(Client::requests::DOWNLOAD) + username + '&' + password + '&' + path;
        sendRawBytes(request);
        std::vector<byte> respond = recieveRawBytes();
        close(sockfd);
        //here is op to change dwnload folder
        std::ofstream fileStream(filename , std::ios::binary | std::ios::trunc);
        fileStream.write(reinterpret_cast<char*>(&respond[0]) , respond.size());
        fileStream.close();
    }

    void uploadFile(std::string path, std::string filename){
        run();
        std::ifstream fileStream(filename , std::ios::binary | std::ios::ate);
        std::ifstream::pos_type fSize = fileStream.tellg();
        fileStream.seekg(0 , std::ios::beg);
        //note: this is NOT the actual fileSize, its just the last index
        //fSize = <actualFileSize> - 1
        if (fSize <= 0)
            throw errors::badFile;
        std::vector<byte> bytes(static_cast<uint64_t >(fSize)),
                          request;
        fileStream.read(reinterpret_cast<char*>(&bytes.at(0)) ,fSize);
        request += static_cast<char>(requests::UPLOAD) + username + '&' + password + '&' + path + filename + '&' + vecToString(bytes);
        sendRawBytes(request);
        std::vector<byte> resp = recieveRawBytes();
        if (vecToNum(resp) != static_cast<int>(errors::noErrors))
            throw static_cast<errors>(resp[0]);
        fileStream.close();
    }
};