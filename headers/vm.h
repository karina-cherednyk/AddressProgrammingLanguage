#ifndef VM_H
#define VM_H


#include <map>
#include <string>
#include "chunk.h"
#include "compiler.h"


enum class InterpretResult {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

#define STACK_MAX 256
class Vm {

    Chunk* chunk{NULL};
    size_t ip;

    Value stack[STACK_MAX];
    size_t stackCount{0};
    Value memory[STACK_MAX];
    size_t memorySize{0};

    Compiler::Parser p;
    Compiler compiler{p};
    std::map<std::string , Value*> pMap;

    void runtimeError(const char* format, ...);


    byte readByte();
    void push(Value value);
    Value pop();
    Value peek(size_t distance);
    InterpretResult setPointer(bool inverse, bool push);
    InterpretResult getPointer();
    Value* stringToPointer(std::string s);

    static bool isFalsey(Value value);
    Value* addToMemory(const Value& value);

    InterpretResult run();
    bool programFinished =  false;

public:
    InterpretResult interpret(const char* source);
    void initVM();
    void freeVM();

};


#endif //VM_H
