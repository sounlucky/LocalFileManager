cmake_minimum_required(VERSION 3.7)
project(FileManager_client)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp menu.hxx menu.cpp menu.h sender.h sender.cpp BasicConnection.cpp BasicConnection.h)
add_executable(FileManager_client ${SOURCE_FILES})