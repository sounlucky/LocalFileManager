//
// Created by unl on 02.04.17.
//
#include <iostream>
#include <fstream>
#include "string.h"
#include "BasicConnection.h"

using std::cout;

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

    vector<string> getFileListing(){

    }

    void run(){
        // <--------------->





        // <--------------->
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

            vector<byte> request = recieveRawBytes(),
                         respond;

            //debugging purposes , rm after
            for (auto &i : request)
                cout<< i;
            cout << std::endl;

            const byte reqLetter = request[0];

            std::fstream dbUsers("dbUsers");
            dbUsers.seekg(0 , std::ios::end);
            int64_t dbUsersSize = dbUsers.tellg();

            dbUsers.seekg(0 , std::ios::beg);
            string dbUsersInformation(dbUsersSize , 'A');

            dbUsers.read(&dbUsersInformation.at(0) , dbUsersSize);

            //ATTENTION! Its closed here!
            dbUsers.close();

            switch  (reqLetter){
                case (byte)Server::requests::REGISTRATION :
                    //request example: Rusername&psswrd
                    if (doesUserExist(request, dbUsersInformation)) {
                        cout << "user already registered " << std::endl;
                        respond += std::to_string((uint8_t) Server::errors ::alreadyRegistered);
                    } else {
                        cout << "new user " <<  std::endl;
                        registrate(request, dbUsers);
                        respond += std::to_string((uint8_t) Server::errors ::noErrors);
                    }
                    break;

                case (byte)Server::requests::LOGIN :
                    //request example: Lusername&password
                    if (doesAccExist(request ,dbUsersInformation)) {
                        cout << "success on login!" << std::endl;
                        respond += std::to_string((uint8_t) Server::errors ::noErrors);
                    }
                    else {
                        cout << "did not log in" << std::endl;
                        respond += std::to_string((uint8_t) Server::errors ::badLogin);
                    }
                    break;

                default:
                    respond += std::to_string((uint8_t) Server::errors ::unknownRequest);
                    break;
            }
            sendRawBytes(respond);
        }
    }

    void registrate(const vector<byte>& request , std::fstream& dbUsers){
        dbUsers.open("dbUsers" , std::ios::app);
        dbUsers.write(reinterpret_cast<const char*>(&request[1]), request.size() - 1);
        dbUsers << std::endl;
        dbUsers.close();
    }

    bool doesAccExist(const vector<byte>& request , const string& fileInfo){
        string line;
        //figure out username
        string::size_type i = 0;
        while ( i < fileInfo.length() ){
            switch (fileInfo[i]){
                case '\n':
                    if (line == vecToString(request).substr(1,request.size()))
                        return true;
                    line = "";
                    break;

                default:
                    line += fileInfo[i];
                    break;
            }
            i++;
        }
        return false;
    }

    bool doesUserExist(const vector<byte>& request , const string& fileInfo){

        string uName;
        //figure out username
        for (auto i = request.begin() + 1 ; i != request.end() ; i++){//we're starting with 2nd symbol coz 1st is meaningLetter
            switch (*i){
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

        while (i < fileInfo.length()){

            switch(fileInfo[i]){
                case '&':
                    //check if he has found one
                    if (currUsername == uName)
                        return true;

                    //dont really need passwords, just skip 'em
                    while ( fileInfo[++i] != '\n')
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
        return false;
    }
};

int main() {
    try {
        Server serv(22321);
        serv.run();
    }
    catch ( Server::errors & er){
        cout<<"Error code "<<(int)er;
        return (int)er;
    }
}