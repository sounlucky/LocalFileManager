//
// Created by unl on 02.04.17.
//
#pragma once
#include <unistd.h>
#include <termios.h>

struct menuPointer {
    bool InMenu;
    uint16_t pos;
};

std::string RawStr(std::string in);

template <uint16_t W, uint16_t H>
class Menu{
public:
    menuPointer Pointer;
    Menu(std::vector<std::string>&, std::vector<std::string>&);
    menuPointer GetInput();
    friend std::string RawStr(std::string);
private:
    enum {
        KEY_UP = 65,
        KEY_DOWN = 66,
        KEY_TAB = 9,
        KEY_ENTER = 10,
        KEY_DELETE = 127
    };
    std::vector<std::string> m_ActualField;
    std::vector<std::string> &m_Linked_Menu;
    std::vector<std::string> &m_Linked_Content;

    void Draw();
    u_char GetCh();
    void Update();
    uint16_t AboveTheCurrent();
    uint16_t AboveMenu();
    uint16_t AboveContent();
    uint16_t ToBeDrawn(std::vector<std::string> &in, uint16_t above);
    bool ChooseAble(std::string in);
    bool InputAble(std::string in);
    std::vector<std::string> &CurrMenu();
    std::vector<std::string> &OthrMenu();
};

#include "menu.hxx"