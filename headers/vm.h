#ifndef VM_H
#define VM_H


#include <map>
#include <string>
#include "chunk.h"
#include "../sources/compiler.h"

enum class InterpretResult {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

enum class VType {POINTER, NUMBER};
struct Val {
    VType type;
    union { double val; const Val* pointTo; } as;
};

void printVal(const Val& x);

#define STACK_MAX 256
class Vm {



    Chunk* chunk{NULL};
    std::vector<byte>::iterator ip;

    Val stack[STACK_MAX];
    size_t stackCount{0};

    Val memory[STACK_MAX];
    size_t memorySize{0};

    Compiler compiler;
    std::map<std::string, Val*> pointerTable;

    void runtimeError(const char* format, ...);
    InterpretResult run();

    byte readByte();
    void push(Val& value);
    Val& pop();
    Val& peek(size_t distance);

    //static bool isFalsey(Value value);
    InterpretResult setPointer(bool  inverse);

public:
    InterpretResult interpret(const char* source);
    void initVM();
    void freeVM();

};


#endif //VM_H
