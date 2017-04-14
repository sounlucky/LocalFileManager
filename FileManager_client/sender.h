//
// Created by unl on 02.04.17.
//
#include "BasicConnection.h"
#include <string.h>

/*
vector<byte> operator+(const vector<byte>& vec, const string& str){
    vector<byte> res(vec.size() + str.length());

    if (vec.size() > 0)
        bcopy( &vec[0], &res[0] , res.size());

    if (str.size() > 0)
        bcopy( &str[0], &res[ vec.size() ] , str.length());

    return res;
}*/

vector<byte>& operator+=(vector<byte>& vec, const string str){
    vec.resize( vec.size() + str.length() );
    if (str.length() > 0)
        bcopy( &str[0], &vec[ vec.size() - str.length() ] , str.length() );

    return vec;
}

string vecToString(vector<byte> vec){
    string res(vec.size(),'\0');
    if (vec.size() > 0)
        bcopy(  &vec[0], &res[0], vec.size() );
    return res;
}

int64_t vecToNum(vector<byte> vec){
    string temp = vecToString(vec);
    return atoi( temp.c_str());//should test std::to_int64 !
}

class Client : public BasicConnection {
public:
    string username,
            password;
    Client(uint32_t inPort , string inHostname){
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

    void registrate(string name, string pass){
        run();
        vector<byte> toSend;
        toSend += (char)Client::requests::REGISTRATION + name + '&' + pass;
        sendRawBytes(toSend);
        vector<byte> respond = recieveRawBytes();
        if (  vecToNum(respond) != (uint8_t) Client::errors::noErrors )
            throw (Client::errors)vecToNum(respond);
        return;
    }

    void login(string name, string pass){
        run();
        vector<byte> toSend;
        toSend += (char)Client::requests::LOGIN + name + '&' + pass;
        sendRawBytes(toSend);
        vector<byte> respond = recieveRawBytes();
        if (  vecToNum(respond) != 0 )
            throw (Client::errors)vecToNum(respond);
        username = name;
        password = pass;
        return;
    }

    vector<string> getFileListing(string path){
        run();
        vector<byte> request;
        request += (char)Client::requests::FILE_LISTING + username + '&' + password + '&' + path;
        sendRawBytes(request);
        vector<byte> respond = recieveRawBytes();
        vector<string> files;
        if (path != "")
            files.push_back("..");
        string temp = "";
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

    void downloadFile(string path){

    }

};


