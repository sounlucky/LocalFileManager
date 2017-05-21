#include <iostream>
#include <vector>

#include "menu.h"
#include "sender.h"

int main() {
    Client* CurrentClient;
    std::string CurrPath = "";
    constexpr int32_t PORT = 14941;
    constexpr uint16_t CLIENT_WIDTH = 70;
    constexpr uint16_t CLIENT_HEIGHT = 15;
    std::vector<std::string> LocalMenu = { "~File Manager" , "~username:" , ">" , "~password:" , ">" , "~host:" , ">localhost" , "connect!" , "registration" , "exit" };
    std::vector<std::string> LocalContent = { "~no data yet" };
    Menu<CLIENT_WIDTH , CLIENT_HEIGHT> ObjMenu(LocalMenu, LocalContent);
    bool EndCycle = false;
    while (!EndCycle)
    {
        menuPointer Pointer = ObjMenu.GetInput();
        try
        {
            if (Pointer.InMenu)
            {
                if (LocalMenu[0] == "~File Manager")
                {
                    enum {
                        OPTION_USERNAME = 2,
                        OPTION_PASSWORD = 4,
                        OPTION_HOSTNAME = 6,
                        OPTION_CONNECT = 7,
                        OPTION_REGISTRATION = 8,
                        OPTION_EXIT = 9
                    };
                    switch (Pointer.pos)
                    {
                        case OPTION_CONNECT:
                            CurrentClient = new Client(PORT, RawStr(LocalMenu[OPTION_HOSTNAME]));
                            CurrentClient->Login(RawStr(LocalMenu[OPTION_USERNAME]), RawStr(LocalMenu[OPTION_PASSWORD]));
                            LocalMenu = {"~welcome", "upload file", "back to menu", "exit"};
                            LocalContent = CurrentClient->GetFileListing(CurrPath);
                            break;
                        case OPTION_REGISTRATION:
                            CurrentClient = new Client(PORT, RawStr(LocalMenu[OPTION_HOSTNAME]));
                            CurrentClient->Registrate(RawStr(LocalMenu[OPTION_USERNAME]), RawStr(LocalMenu[OPTION_PASSWORD]));
                            LocalMenu = {"~welcome", "upload file", "back to menu", "exit"};
                            LocalContent = CurrentClient->GetFileListing(CurrPath);
                            break;
                        case OPTION_EXIT:
                            EndCycle = true;
                            break;
                        default:
                            break;
                    }
                } else if (LocalMenu[0] == "~welcome")
                {
                    enum {
                        OPTION_UPLOAD_FILE = 1,
                        OPTION_BACK_TO_MENU = 2,
                        OPTION_EXIT = 3
                    };
                    switch (Pointer.pos)
                    {
                        case OPTION_UPLOAD_FILE:
                            LocalMenu = { "~uploading" , "~type filename:" , ">" , "upload" , "back" };
                            break;
                        case OPTION_BACK_TO_MENU:
                            CurrPath = "";
                            LocalMenu = {"~File Manager", "~username:", ">", "~password:", ">", "~host:", ">localhost",
                                         "connect!", "registration", "exit"};
                            LocalContent = {"~no data yet"};
                            break;
                        case OPTION_EXIT:
                            EndCycle = true;
                            break;
                        default:
                            break;
                    }
                } else if (LocalMenu[0] == "~uploading")
                {
                    enum {
                        OPTION_FILENAME = 2,
                        OPTION_UPLOAD = 3,
                        OPTION_BACK = 4
                    };
                    switch (Pointer.pos){
                        case OPTION_UPLOAD:
                            CurrentClient->UploadFile(CurrPath , RawStr(LocalMenu[OPTION_FILENAME]));
                            break;
                        case OPTION_BACK:
                            LocalMenu = {"~welcome" , "upload file" , "back to menu" , "exit"};
                            break;
                        default:
                            break;
                    }
                } else if (LocalMenu[0] == "Back to menu")
                {
                    CurrPath = "";
                    LocalMenu = {"~File Manager", "~username:", ">", "~password:", ">", "~host:", ">localhost",
                                 "connect!", "registration", "exit"};
                    LocalContent = {"~no data yet"};
                }
            } else {
                if (LocalContent[Pointer.pos].back() == '/')
                {
                    CurrPath += LocalContent[Pointer.pos];
                    LocalContent = CurrentClient->GetFileListing(CurrPath);
                    ObjMenu.Pointer.pos = 0;
                }
                else if (LocalContent[Pointer.pos] == "..")
                {
                    do {
                        CurrPath.resize(CurrPath.size() - 1);
                        std::cout<< CurrPath <<'\n';
                    }
                    while (CurrPath.back() != '/' && CurrPath.size() > 0);
                    LocalContent = CurrentClient->GetFileListing(CurrPath);
                    ObjMenu.Pointer.pos = 0;
                } else
                    CurrentClient->DownloadFile(CurrPath + LocalContent[Pointer.pos] , LocalContent[Pointer.pos]);
            }
        } catch (Client::errors e)
        {
            delete CurrentClient;
            CurrPath = "";
            LocalMenu = {"Back to menu"};
            LocalContent = { ( "Something bad happened. Error code : " + std::to_string( static_cast<int>(e) ) ) };
        }
    }
    return 0;
}