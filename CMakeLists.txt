cmake_minimum_required(VERSION 3.21)
project(AddressProgrammingLanguage)

set(CMAKE_CXX_STANDARD 14)

add_executable(AddressProgrammingLanguage main.cpp sources/scanner.cpp headers/scanner.h)
