//
// Created by unl on 02.04.17.
//
#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include <unistd.h>
#include <termios.h>


template<uint16_t width, uint16_t height>
class menu{
public:

    struct menuPointer {
        bool inMenu;
        uint16_t pos;
    };

    menuPointer pointer;

    menu(std::vector<std::string> &inMenu, std::vector<std::string> &inContent) :
            linked_menu(inMenu), linked_content(inContent) {
        std::string border(width, '_'),
                inside(width, ' ');
        inside[0] = '|';
        inside[width - 1] = '|';
//filling first "layer"
        actualField.push_back(border);
//middle
        int currMenuPos = 0, currContentPos = 0;
        for (uint16_t i = 0; i < height - 2; i++) {
            actualField.push_back(inside);
            if (i % 2) actualField[i][width / 3] = '|';
        }
//last
        border[0] = '|';
        border[width - 1] = '|';
        actualField.push_back(border);

//create pointer
        pointer = {1, 0};
        for (uint16_t i = 0; i < inMenu.size(); i++)
            if (chooseAble(inMenu[i])) {
                pointer.pos = i;
                break;
            }
    }

    ~menu() = default;

    menuPointer getInput() {
        //if menu has changed there's a chance of getting out of range
        if (currMenu().size() < pointer.pos)
            for (uint16_t i = 0; i < currMenu().size(); i++)
                if (chooseAble(currMenu()[i])) {
                    pointer.pos = i;
                    break;
                }
        draw();

        unsigned char input;
        while ((input = getCh()) != ENTER) {
            switch (input) {
                case UP:
                    for (uint16_t i = (uint16_t) (pointer.pos == 0 ? currMenu().size() - 1 : (int8_t) pointer.pos -
                                                                                             1 % currMenu().size());
                         i != pointer.pos;
                         i = (uint16_t) (i == 0 ? currMenu().size() - 1 : (int8_t) i - 1 % currMenu().size())) {
                        if (chooseAble(currMenu()[i])) {
                            pointer.pos = i;
                            break;
                        }
                    }
                    break;

                case DOWN:
                    for (uint16_t i = (uint16_t) ((pointer.pos + 1) % currMenu().size());
                         i != pointer.pos; i = (uint16_t) (
                            (i + 1) % currMenu().size())) {
                        if (chooseAble(currMenu()[i])) {
                            pointer.pos = i;
                            break;
                        }
                    }
                    break;
                case TAB:
                    for (uint16_t i = 0; i < othrMenu().size(); i++)
                        if (chooseAble(othrMenu().at(i))) {
                            pointer.inMenu = !pointer.inMenu;
                            pointer.pos = i;
                            break;
                        }
                default:
                    //probably input
                    if (inputAble(currMenu()[pointer.pos])) {
                        if (('0' <= input && input <= '9') || ('A' <= input && input <= 'Z') ||
                            ('a' <= input && input <= 'z') || (input == '.')) {
                            currMenu()[pointer.pos] += input;
                        } else if (input == DELETE && currMenu()[pointer.pos].length() > 1) {//delete
                            currMenu()[pointer.pos].pop_back();
                        }
                    }
            }
            draw();
        }
        return pointer;
    }

    static std::string rawStr(std::string in) {
        return (in[0] == '~') || (in[0] == '>') ? in.substr(1, in.length()) : in;
    }

private:
    static constexpr uint16_t   UP = 65,
                                DOWN = 66,
                                TAB = 9,
                                ENTER = 10,
                                DELETE = 127;
    void draw() {
        system("clear");
        update();
        for (size_t i = 0; i < height; i++) {
            for (size_t g = 0; g < width; g++)
                std::cout << actualField[i][g];
            std::cout << '\n';
        }
    }

    u_char getCh() {
        u_char buf = 0;
        struct termios old = {0};
        fflush(stdout);
        if (tcgetattr(0, &old) < 0) {
            perror("tcsetattr()");
        }
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0) {
            perror("tcsetattr ICANON");
        }
        if (read(0, &buf, 1) < 0) {
            perror("read()");
        }
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0) {
            perror("tcsetattr ~ICANON");
        }
        return buf;
    }

    void update() {
        for (uint16_t i = 1; i < height - 1; i++)
            for (uint16_t j = 1; j < width - 1; j++)
                if ((i % 2) && j == width / 3)
                    actualField[i][j] = '|';
                else actualField[i][j] = ' ';
        //Menu
        uint16_t iterator = aboveMenu();
        for (uint16_t i = 1;//drawing from 1
             iterator < tobeDrawn(linked_menu, aboveMenu()) + aboveMenu();
             i += height / (tobeDrawn(linked_menu, aboveMenu()) + 1)) {
            if (pointer.inMenu && iterator == pointer.pos) {
                actualField[i][1] = '>';
            }
            for (uint16_t g = 0;
                 g < rawStr(linked_menu[iterator]).length() && g < width / 3 - 2;
                 g++) {
                actualField[i][g + 2] = rawStr(linked_menu[iterator])[g];
            }
            iterator++;
        }
        //Content
        iterator = aboveContent();
        for (uint16_t i = 1;
             iterator < tobeDrawn(linked_content, aboveContent()) + aboveContent();
             i += height / (tobeDrawn(linked_content, aboveContent()) + 1)) {
            if (!pointer.inMenu && iterator == pointer.pos) {
                actualField[i][width / 3 + 1] = '>';
            }
            for (uint16_t g = 0;
                 g < rawStr(linked_content[iterator]).length() && g < 2 * width / 3 - 2;
                 g++) {
                actualField[i][g + width / 3 + 2] = rawStr(linked_content[iterator])[g];
            }
            iterator++;
        }
    }

    //inlining stuff
    inline uint16_t aboveTheCurrent() {
        return (uint16_t) (pointer.pos / (height - 2) * (height - 2));
    }

    inline uint16_t aboveMenu() {
        return (uint16_t) (pointer.inMenu ? aboveTheCurrent() : 0);
    }

    inline uint16_t aboveContent() {
        return (uint16_t) (!pointer.inMenu ? aboveTheCurrent() : 0);
    }

    inline uint16_t tobeDrawn(std::vector<std::string> &in, uint16_t above) {
        return (uint16_t) ((uint16_t) in.size() - above > height - 2 ?
                           (unsigned long) (height - 2) :
                                in.size() - above);
    }

    inline bool chooseAble(std::string in) {
        return in[0] != '~';
    }

    inline bool inputAble(std::string in) {
        return in[0] == '>';
    }

    inline std::vector<std::string> &currMenu() {
        return (pointer.inMenu ? linked_menu : linked_content);
    }

    inline std::vector<std::string> &othrMenu() {
        return (!pointer.inMenu ? linked_menu : linked_content);
    }

    std::vector<std::string> actualField;
    std::vector<std::string> &linked_menu;
    std::vector<std::string> &linked_content;
};