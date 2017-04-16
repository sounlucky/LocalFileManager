#include "menu.h"
#include "sender.h"

int main() {
    Client* client;
    //
    /*
    try {
        client = new Client(2312, "localhost");
        client->login("user", "pass");
        client->uploadFile("", "a");
    }catch (Client::errors e) {
        return static_cast<int8_t>(e);
    }*/
    //
    string currPath = "";
    constexpr int32_t PORT = 2312;
    constexpr uint16_t  CLIENT_WIDTH = 70,
                        CLIENT_HEIGHT = 15;
    vector<string> localMenu = { "~File Manager" , "~username:" , ">user" , "~password:" , ">pass" , "~host:" , ">localhost" , "connect!" , "registration" , "exit" };
    vector<string> localContent = { "~no data yet" };
    menu<CLIENT_WIDTH , CLIENT_HEIGHT> objMenu(localMenu, localContent);
    bool endCycle = false;
    while (!endCycle){
        menu<CLIENT_WIDTH , CLIENT_HEIGHT>::menuPointer pointer = objMenu.getInput();
        try {
            if (pointer.inMenu) {
                if (localMenu[0] == "~File Manager") {
                    constexpr uint8_t   USERNAME = 2,
                                        PASSWORD = 4,
                                        HOSTNAME = 6,
                                        CONNECT = 7,
                                        REGISTRATION = 8,
                                        EXIT = 9;
                    switch (pointer.pos) {
                        case CONNECT:
                            client = new Client(PORT, objMenu.rawStr(localMenu[HOSTNAME]));
                            client->login(
                                    menu<CLIENT_WIDTH, CLIENT_HEIGHT>::rawStr(localMenu[USERNAME]),//just strings "name"
                                    menu<CLIENT_WIDTH, CLIENT_HEIGHT>::rawStr(localMenu[PASSWORD]));//and "pass"
                            localMenu = {"~welcome", "upload file", "back to menu", "exit"};
                            localContent = client->getFileListing(currPath);
                            break;
                        case REGISTRATION:
                            client = new Client(PORT, objMenu.rawStr(localMenu[HOSTNAME]));
                            client->registrate(menu<CLIENT_WIDTH, CLIENT_HEIGHT>::rawStr(localMenu[USERNAME]), //same
                                               menu<CLIENT_WIDTH, CLIENT_HEIGHT>::rawStr(                      //as above
                                                       localMenu[PASSWORD]));
                            localMenu = {"~welcome", "upload file", "back to menu", "exit"};
                            localContent = client->getFileListing(currPath);
                            break;
                        case EXIT:
                            endCycle = true;
                            break;
                        default:
                            break;
                    }
                } else if (localMenu[0] == "~welcome") {
                    constexpr uint8_t   UPLOAD_FILE = 1,
                                        BACK_TO_MENU = 2,
                                        EXIT = 3;
                    switch (pointer.pos) {
                        case UPLOAD_FILE:
                            localMenu = { "~uploading" , "~type filename:" , ">" , "upload" , "back" };
                            break;
                        case BACK_TO_MENU:
                            currPath = "";
                            localMenu = {"~File Manager", "~username:", ">", "~password:", ">", "~host:", ">localhost",
                                         "connect!", "registration", "exit"};
                            localContent = {"~no data yet"};
                            break;
                        case EXIT:
                            endCycle = true;
                            break;
                        default:
                            break;
                    }
                }else if (localMenu[0] == "~uploading") {
                    constexpr uint8_t   FILENAME = 2,
                                        UPLOAD = 3,
                                        BACK = 4;
                    switch (pointer.pos){
                        case UPLOAD:
                            client->uploadFile(currPath , menu<CLIENT_WIDTH , CLIENT_HEIGHT>::rawStr(localMenu[FILENAME]));
                            break;
                        case BACK:
                            localMenu = {"~welcome" , "upload file" , "back to menu" , "exit"};
                            break;
                        default:
                            break;
                    }
                }else if (localMenu[0] == "Back to menu") {
                    currPath = "";
                    localMenu = {"~File Manager", "~username:", ">", "~password:", ">", "~host:", ">localhost",
                                 "connect!", "registration", "exit"};
                    localContent = {"~no data yet"};
                }
            } else{
                //in case the pointer is not in menu
                if (localContent[pointer.pos].back() == '/'){
                    //its folder
                    currPath += localContent[pointer.pos];
                    localContent = client->getFileListing(currPath);
                    objMenu.pointer.pos = 0;
                }
                else if (localContent[pointer.pos] == ".."){
                    do{
                        currPath.resize( currPath.size() - 1);
                        std::cout<<currPath<<'\n';
                    }
                    while (currPath.back() != '/' && currPath.size() > 0);
                    localContent = client->getFileListing(currPath);
                    objMenu.pointer.pos = 0;
                }else{
                    client->downloadFile(currPath + localContent[pointer.pos] , localContent[pointer.pos] );
                }
            }
        } catch (Client::errors e){
            delete client;
            currPath = "";
            localMenu = {"Back to menu"};
            localContent = { ( "Something bad happened. Error code : " + std::to_string( static_cast<int>(e) ) ) };
        }
    }
    return 0;
}