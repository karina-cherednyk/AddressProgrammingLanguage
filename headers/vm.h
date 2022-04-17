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
    std::vector<byte>::iterator ip;

    Value stack[STACK_MAX];
    size_t stackCount{0};
    Value memory[STACK_MAX];
    size_t memorySize{0};

    Compiler compiler;
    std::map<const char*, Value> pMap;

    void runtimeError(const char* format, ...);
    InterpretResult run();

    byte readByte();
    void push(Value value);
    Value pop();
    Value peek(size_t distance);
    InterpretResult setPointer(bool inverse);


    static bool isFalsey(Value value);
    Value* addToMemory(const Value& value);

public:
    InterpretResult interpret(const char* source);
    void initVM();
    void freeVM();

};


#endif //VM_H
