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

using std::cout;

vector<byte> &operator+=(vector<byte> &vec, const string str) {
    vec.resize(vec.size() + str.length());

    if (str.length() > 0)
        bcopy(&str[0], &vec[vec.size() - str.length()], str.length());

    return vec;
}

string vecToString(vector<byte> vec) {
    string res(vec.size(), 'v');
    if (vec.size() > 0)
        bcopy(&vec[0], &res[0], vec.size());
    return res;
}

class Server : public BasicConnection {

public:

    Server(int32_t inPort) {
        port = inPort;
    }

    void run() {
        int32_t n, selfsockfd;
        sockaddr_in cli_addr;
        selfsockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (selfsockfd < 0)
            throw Server::errors::openingSocket;
        bzero((char *) &address, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(static_cast<uint16_t>(port));
        if (bind(selfsockfd, (sockaddr *) &address, sizeof(address)) < 0)
            throw Server::errors::binding;
        listen(selfsockfd, 10);
        socklen_t clilen = sizeof(cli_addr);
        while (1) {
            printf("\n waiting... \n");
            sockfd = accept(selfsockfd, (struct sockaddr *) &cli_addr, &clilen);
            vector<byte> request = recieveRawBytes(),
                    respond;
            //debugging purposes , rm after
            for (auto &i : request)
                cout << i;
            cout << std::endl;
            const byte reqLetter = request[0];
            std::fstream dbUsers("dbUsers");
            dbUsers.seekg(0, std::ios::end);
            int64_t dbUsersSize = dbUsers.tellg();
            dbUsers.seekg(0, std::ios::beg);
            string dbUsersInformation(dbUsersSize, '\0');
            dbUsers.read(&dbUsersInformation.at(0), dbUsersSize);
            //ATTENTION! Its closed here!
            dbUsers.close();
            switch (reqLetter) {
                case (byte) Server::requests::REGISTRATION :
                    //request example: R<username>&<psswrd>
                    if (doesUserExist(request, dbUsersInformation)) {
                        respond += std::to_string((uint8_t) Server::errors::alreadyRegistered);
                    } else {
                        registrate(request, dbUsers);
                        respond += std::to_string((uint8_t) Server::errors::noErrors);
                    }
                    break;
                case (byte) Server::requests::LOGIN :
                    //request example: L<username>&<password>
                    if (doesAccExist(request, dbUsersInformation))
                        respond += std::to_string((uint8_t) Server::errors::noErrors);
                    else
                        respond += std::to_string((uint8_t) Server::errors::badLogin);
                    //responce is obv 1 byte
                    break;
                case (byte) Server::requests::FILE_LISTING:
                    //request example: F<username>&<password>&<folder1/folder2/>
                    respond += sendFileListing(request, dbUsersInformation);
                    //responce is obv 1 byte
                    break;
                case (byte) Server::requests::UPLOAD:
                    //request example: U<username>&<password>&<folder1/folder2/file>&<bytes>
                    //
                    break;
                case (byte) Server::requests::DOWNLOAD:
                    //request example: D<username>&<password>&<folder1/folder2/file>
                    respond = getFile(request , dbUsersInformation);
                    //responce is just a file
                    break;

                default:
                    respond += std::to_string((uint8_t) Server::errors::unknownRequest);
                    break;
            }
            std::cout << "resp : " << vecToString(respond);
            sendRawBytes(respond);
        }
    }

    vector<byte> getFile(vector<byte> request, string& dbUsersInformation) {
        //todo ,not working
        string path = parse(request , dbUsersInformation);
        std::ifstream file(path, std::ios::binary);

        // Stop eating new lines in binary mode!!!
        file.unsetf(std::ios::skipws);

        // get its size:
        std::streampos fileSize;

        file.seekg(0, std::ios::end);
        fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        // reserve capacity
        std::vector<byte> vec;
        vec.reserve(fileSize);

        // read the data:
        vec.insert(vec.begin(),
                   std::istream_iterator<byte>(file),
                   std::istream_iterator<byte>());

        return vec;
    }


    std::string exec(const char *cmd) {
        constexpr int16_t BUFFER_LENGTH = 0xFF;
        std::array<char, BUFFER_LENGTH> buffer;
        string res;
        std::shared_ptr<FILE> cmdReader(popen(cmd, "r"), pclose);
        while (!feof(cmdReader.get())) {
            if (fgets(buffer.data(), BUFFER_LENGTH, cmdReader.get()) != NULL) {
                res += buffer.data();
            }
        }
        return res;
    }

    string parse(vector<byte> request, string& dbUsersInformation){
        // function converts
        // Auser&pass&text -> text
        // and also checking accExistence
        vector<byte> userData;   // user&pass
        string rawData;          // text
        vector<string>::size_type i = 0;
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
            throw std::to_string((uint8_t) Server::errors::badLogin);
        return rawData;
    }

    string sendFileListing(vector<byte> &request, string &dbUsersInformation) {
        string path = parse(request , dbUsersInformation),
            folders = exec(string("cd  fileStorage/" + path + "; ls -d */").c_str()),
                files = exec(string("cd  fileStorage/" + path + "; ls -p | grep -v /").c_str());
        if (folders == "ls: cannot access '*/': No such file or directory")
            return files;
        else
            return folders + files;

}


    void registrate(const vector<byte> &request, std::fstream &dbUsers) {
        dbUsers.open("dbUsers", std::ios::app);
        dbUsers.write(reinterpret_cast<const char*>(&request[1]), request.size() - 1);
        dbUsers << std::endl;
        dbUsers.close();
    }

    bool doesAccExist(const vector<byte> &request, const string &fileInfo) {
        string line;
        string::size_type i = 0;
        while (i < fileInfo.length()) {
            switch (fileInfo[i]) {
                case '\n':
                    //obv beginning of the new line
                    if (line == vecToString(request).substr(1, request.size())) {
                        cout << "account does exist" << std::endl;
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
        cout << "account doesnt exist" << std::endl;
        return false;
    }

    bool doesUserExist(const vector<byte> &request, const string &fileInfo) {
        string uName;
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
        string currUsername;
        string::size_type i = 0;
        while (i < fileInfo.length()) {
            switch (fileInfo[i]) {
                case '&':
                    //check if he has found one
                    if (currUsername == uName) {
                        cout << " user does exist" << std::endl;
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
        cout << "user does not exist" << std::endl;
        return false;
    }
};


int main() {
    try {
        constexpr int32_t PORT = 1810;
        Server serv(PORT);
        serv.run();
    }
    catch (Server::errors &er) {
        cout << "Error code " << (int16_t) er;
        return (int) er;
    }
}