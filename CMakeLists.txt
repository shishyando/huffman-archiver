project(archiver)

cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -std=c++20")

set(SRC_LIST ArgsProcessing.h ArgsProcessing.cpp BitIO.h BitIO.cpp HuffmanCodec.h HuffmanCodec.cpp HuffmanTree.h HuffmanTree.cpp LeftistHeap.h)

add_executable(archiver main.cpp ${SRC_LIST})
add_executable(test_archiver catch.hpp catch_main.cpp tests.cpp ${SRC_LIST})