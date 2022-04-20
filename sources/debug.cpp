#include <cstdio>
#include <iostream>
#include "../headers/debug.h"

void disassembleInstructions(const Chunk* chunk){
#define OP_CASE(name)  case(name): {std::cout << #name << '\n'; break;}
    for(auto ip = chunk->code.begin(); ip != chunk->code.end(); ){
        switch (*ip++) {
            OP_CASE(OP_RETURN)
            OP_CASE(OP_NEGATE)
            OP_CASE(OP_ADD)
            OP_CASE(OP_SUBTRACT)
            OP_CASE(OP_MULTIPLY)
            OP_CASE(OP_DIVIDE)
            OP_CASE(OP_NOT)
            OP_CASE(OP_LESS)
            OP_CASE(OP_EQUAL)
            OP_CASE(OP_GREATER)
            OP_CASE(OP_TRUE)
            OP_CASE(OP_FALSE)
            OP_CASE(OP_PRINT)
            OP_CASE(OP_POP)
            OP_CASE(OP_SET_POINTER)
            OP_CASE(OP_GET_POINTER)
            OP_CASE(OP_SET_POINTER_INVERSE)
            OP_CASE(OP_PART_END)
            OP_CASE(OP_EXCHANGE)
            case OP_JUMP_IF_FALSE:
            case OP_JUMP: {
                std::cout << "OP_JUMP ";
                if((OpCode)*(ip-1) == OP_JUMP_IF_FALSE) std::cout << "if false ";
                std::cout << (size_t)*ip++ ;
                std::cout << std::endl;
                break;
            }
            case OP_CONSTANT:{
                std::cout << "OP_CONSTANT \t";
                chunk->constants.at(*ip++).printValue();
                std::cout << std::endl;
                break;
            }
            default: std::cout << "Unknown OP :\t"  << *(ip-1) << std::endl;;
        }
    }
#undef OP_CASE
}