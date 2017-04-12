#include "menu.h"
#include "sender.h"

int main() {

    Client *client;

    constexpr uint32_t PORT = 102010;


    client =  new Client(PORT, "localhost");
    client->run();
    client->login("user", "pass");
    auto n = client->getFileListing("");

    constexpr uint16_t  CLIENT_WIDTH = 70;
    constexpr uint16_t CLIENT_HEIGHT = 15;

    vector<string> localMenu = { "~File Manager" , "~username:" , ">" , "~password:" , ">" , "~host:" , ">localhost" , "connect!" , "registration" , "exit" };
    vector<string> localContent = { "~no data yet" };

    menu< CLIENT_WIDTH , CLIENT_HEIGHT> objMenu(localMenu, localContent);

    bool endCycle = false;

    while (!endCycle){

        menu<CLIENT_WIDTH, CLIENT_HEIGHT>::menuPointer pointer = objMenu.getInput();


        vector<string> vector1;

        try {

            if (localMenu[0] == "~File Manager"){

                constexpr uint8_t     USERNAME = 2;
                constexpr uint8_t     PASSWORD = 4;
                constexpr uint8_t     HOSTNAME = 6;
                constexpr uint8_t      CONNECT = 7;
                constexpr uint8_t REGISTRATION = 8;
                constexpr uint8_t         EXIT = 9;

                    switch (pointer.pos) {

                        case CONNECT:
                            client = new Client(PORT , objMenu.rawStr(localMenu[HOSTNAME]));
                            client->run();
                            client->login(menu<CLIENT_WIDTH ,CLIENT_HEIGHT>::rawStr(localMenu[USERNAME]),
                                          menu<CLIENT_WIDTH , CLIENT_HEIGHT>::rawStr(localMenu[PASSWORD]));
                            localMenu = {"welcome" , "upload file" , "back to menu" , "exit" };

                            std::cout<< '\n' << client->username <<' '<<client->password;

                             client->getFileListing("CMakeFiles/");

                            for (auto i: vector1)
                                std::cout<< '\n' << i;
                            break;

                        case REGISTRATION:
                            client = new Client(PORT , objMenu.rawStr(localMenu[HOSTNAME]));
                            client->run();
                            client->registrate(menu<CLIENT_WIDTH , CLIENT_HEIGHT>::rawStr(localMenu[USERNAME]),
                                               menu<CLIENT_WIDTH , CLIENT_HEIGHT>::rawStr(localMenu[PASSWORD]));
                            localMenu = {"welcome" , "upload file" , "back to menu" , "exit" };
                            client->getFileListing("");
                            break;

                        case EXIT:
                            endCycle = true;
                            break;

                        default:
                            break;
                    }

            } else if (localMenu[0] == "Back to menu"){
                localMenu = { "~File Manager" , "~username:" , ">" , "~password:" , ">" , "~host:" , ">localhost" , "connect!" , "registration" , "exit" };
                localContent = { "~no data yet" };
            }
        } catch (Client::errors e){

            delete client;

            localMenu = {"Back to menu"};
            localContent = { ( "Something bad happened. Error code : " + std::to_string( static_cast<int>(e) ) ) };
        }
    }
    return 0;
}