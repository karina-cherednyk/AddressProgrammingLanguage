#ifndef VM_H
#define VM_H


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
    Value* stackTop{stack};
    Compiler compiler;

    void runtimeError(const char* format, ...);
    InterpretResult run();

    byte readByte();
    void push(Value value);
    Value pop();
    Value peek(size_t distance);

    void initVM();
    void freeVM();

    static bool isFalsey(Value value);

public:
    InterpretResult interpret(const char* source);



};


#endif //VM_H
