cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(chess-engine)

add_executable(chess-engine src/main.cpp src/board.cpp)