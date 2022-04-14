#include <cstdarg>
#include <cstdio>
#include <cassert>
#include "../headers/vm.h"

void Vm::initVM() {
    stackTop = stack;
}

void Vm::freeVM() {

}

void Vm::runtimeError(const char* format, ...){
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);

    size_t instruction = ip - chunk->code.begin() - 1;
    int line = chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script \n", line);
    stackTop = stack;
}

void Vm::push(Value value) {
    *(stackTop++) = value;
}

Value Vm::pop() {
    return *(--stackTop);
}

Value Vm::peek(size_t distance){
    return  *(stackTop - 1 - distance);
}

byte Vm::readByte() {
    return *ip++;
}

InterpretResult Vm::run() {
#define CHECK_NEXT_NUMBER(pos) \
    if(peek(pos).type != ValueType::NUMBER){   \
        runtimeError("Expected number.");      \
        return InterpretResult::RUNTIME_ERROR; \
    }                      \

#define BINARY_OP(op) \
    do {                         \
                CHECK_NEXT_NUMBER(0);        \
                CHECK_NEXT_NUMBER(1);        \
                double b = pop().as.number;  \
                double a = pop().as.number;  \
                push(Value(a op b));         \
    } while(false)


    for(ip = chunk->code.begin(); ip != chunk->code.end(); ){

        switch (*ip = readByte()) {
            case OP_RETURN:
                pop().printValue();
                printf("\n");
                return InterpretResult::OK;
            case OP_CONSTANT: {
                Value constant = chunk->constants[readByte()];
                push(constant);
                break;
            }
            case OP_TRUE:
                push(Value(true)); break;
            case OP_FALSE:
                push(Value(false)); break;
            case OP_NEGATE:
                CHECK_NEXT_NUMBER(0);
                push(Value(-(pop()).as.number));
                break;
            case OP_NOT:
                push(Value(isFalsey(pop()))); break;
            case OP_ADD:
                BINARY_OP(+); break;
            case OP_SUBTRACT:
                BINARY_OP(-); break;
            case OP_MULTIPLY:
                BINARY_OP(*); break;
            case OP_DIVIDE:
                BINARY_OP(/); break;
            case OP_LESS:
                BINARY_OP(<); break;
            case OP_EQUAL:
            {
                Value b = pop();
                Value a = pop();
                push(Value(a == b));
            }
            case OP_GREATER:
                BINARY_OP(>); break;
        }
    }
    runtimeError("No return statement");
    return InterpretResult::RUNTIME_ERROR;
}


InterpretResult Vm::interpret(const char *source) {
    Chunk codeChunk;
    if(!compiler.compile(source, &codeChunk)) return InterpretResult::COMPILE_ERROR;
    this->chunk = &codeChunk;
    InterpretResult result = run();
    this->chunk = NULL;
    return result;
}

bool Vm::isFalsey(Value value) {
    assert(value.type != ValueType::POINTER);
    return value.type == ValueType::NUMBER ? value.as.number == 0 : !value.as.boolean;
}





