#include "menu.h"
#include "sender.h"

int main() {
    //Client client(22901, "localhost");

    string text = "simpleText";
    vector<byte> res(2);
    res.push_back('c');
    res += (string)"back";
    string ms = string(res);

    /*
    vector<string> localMenu = {"~File Manager", "~login:", ">", "~password:", ">", "~host:", ">localhost", "connect!",
                                "registration", "exit"};
    vector<string> localContent = {"~no data yet!"};

    menu<70, 8> objMenu(localMenu, localContent);

    bool contCycle = true;

    while (contCycle) {
        menuPointer pointer = objMenu.getInput();

        if (localMenu[0] == "~File Manager") {
            constexpr uint8_t CONNECT = 7;
            constexpr uint8_t REGISTRATION = 8;
            constexpr uint8_t EXIT = 9;

            switch (pointer.pos) {
                case CONNECT:
                    localMenu = {"~options", "upload", "create folder", "back to menu"};
                    localContent = {"~files", "file1", "file2", "file3"};
                    break;

                case REGISTRATION:
                    localMenu = {"~registration", "~name:", ">", "~password:", ">", "reg me in!", "back to menu"};
                    localContent = {"~no data yet!"};
                    break;

                case EXIT:
                    contCycle = false;
                    break;

                default:
                    break;
            }
        } else if (localMenu[0] == "~options") {
            constexpr uint8_t UPLOAD = 1;
            constexpr uint8_t CREATE_FOLDER = 2;
            constexpr uint8_t BACK_TO_MENU = 3;

            switch (pointer.pos) {
                case UPLOAD://upload
                    break;

                case CREATE_FOLDER://create folder
                    break;

                case BACK_TO_MENU://back to menu
                    localMenu = {"~File Manager", "~login:", ">", "~password:", ">", "~host:",
                                 ">localhost", "connect!", "registration", "exit"};
                    localContent = {"~no data yet!"};
                    break;

                default:
                    break;
            }
        } else if (localMenu[0] == "~registration") {
            constexpr uint8_t REG_ME_IN = 5;
            constexpr uint8_t BACK_TO_MENU = 6;
            constexpr uint8_t EXIT = 9;
            switch (pointer.pos) {
                case REG_ME_IN:
                    break;

                case BACK_TO_MENU:
                    localMenu = {"~File Manager", "~login:", ">", "~password:", ">", "host:",
                                 ">localhost", "connect!", "registration", "exit"};
                    localContent = {"~no data yet!"};
                    break;

                default:
                    break;
            }
        }
    }*/
    return 0;
}