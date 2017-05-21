//
// Created by unl on 02.04.17.
//
#include <iostream>
#include <vector>

#include "menu.h"

std::string RawStr(std::string In) {
    return (In[0] == '~') || (In[0] == '>') ? In.substr(1, In.length()) : In;
}




