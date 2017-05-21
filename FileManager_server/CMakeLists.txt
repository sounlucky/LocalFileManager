cmake_minimum_required(VERSION 3.7)
project(FileManager_server)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp BasicConnection.cpp BasicConnection.h Server.cpp Server.h)
add_executable(FileManager_server ${SOURCE_FILES})