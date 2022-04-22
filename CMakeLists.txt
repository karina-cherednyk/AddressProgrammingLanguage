cmake_minimum_required(VERSION 3.21)
project(AddressProgrammingLanguage)

set(CMAKE_CXX_STANDARD 14)

add_executable(AddressProgrammingLanguage main.cpp sources/scanner.cpp headers/scanner.h sources/compiler.cpp headers/compiler.h sources/chunk.cpp headers/chunk.h sources/vm.cpp headers/vm.h sources/debug.cpp headers/debug.h sources/parser.cpp sources/compiler2.cpp)
