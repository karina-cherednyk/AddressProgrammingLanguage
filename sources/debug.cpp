#include <cstdio>
#include <iostream>
#include "../headers/debug.h"
using namespace std;
void disassembleInstructions(const Chunk* chunk){
    cout << "Labels:" << endl;
    for(auto i = chunk->labelMap.begin(); i != chunk->labelMap.end(); i++){
        cout << i->first << ":\t" << i->second << endl;
    }

    cout << " ---" << endl;
#define OP_CASE(name)  case(name): {cout << #name << '\n'; break;}
    for(int i = 0; i < chunk->code.size(); i++){
        cout << '[' << i << "]\t";
        switch (chunk->code[i]) {
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
            OP_CASE(OP_JUMP_IF_FALSE_TO_LABEL)
            OP_CASE(OP_GET_LABEL)
            OP_CASE(OP_SET_POINTER_WITHOUT_PUSH)
            case OP_JUMP_IF_FALSE:
            case OP_JUMP: {
                cout << "OP_JUMP ";
                if((OpCode)chunk->code[i] == OP_JUMP_IF_FALSE) cout << "if false ";
                cout << (size_t)chunk->code[++i] ;
                cout << endl;
                break;
            }
            case OP_CONSTANT:{
                cout << "OP_CONSTANT \t";
                chunk->constants.at(chunk->code[++i]).printValue();
                cout << endl;
                break;
            }
            default: cout << "Unknown OP :\t"  << chunk->code[i] << endl;;
        }
    }
#undef OP_CASE
}