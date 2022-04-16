#include <cstdarg>
#include <cstdio>
#include <cassert>
#include "../headers/vm.h"

void Vm::initVM() {
    stackCount = 0;
}

void Vm::freeVM() {
    freeStrings();
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
    stackCount = 0;
}

void Vm::push(const Value value) {
    stack[stackCount++] = value;
}

Value Vm::pop() {
    return stack[--stackCount];
}

Value Vm::peek(size_t distance){
    return  stack[stackCount - 1 - distance];
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
                return InterpretResult::OK;
            case OP_PRINT:
                pop().printValue();
                printf("\n");
                break;
            case OP_POP:
                pop();
                constants[1];
                break;

            case OP_SET_POINTER: {
                Value rval = pop();
                Value lval = pop();
                const char* name;
                if(lval.type == ValueType::NUMBER) {
                        name = addNumString(lval.as.number);
                } else if(lval.type == ValueType::STRING){
                        name = lval.as.string;
                } else {
                    runtimeError("Expected pointers only of string or number");
                    return InterpretResult::RUNTIME_ERROR;
                }
                if(rval.type == ValueType::STRING) {
                    const char* rname =  rval.as.string;
                    if(pointerTable.find(rname) == pointerTable.end()){
                        pointerTable[rname] = Value();
                       // runtimeError("Expected pointer under name %s", rval.as.string);
                       // return InterpretResult::RUNTIME_ERROR;
                    }
                    pointerTable[name] = Value(&pointerTable.at(rname));

                }
                else {
                    constants[constantsSize++] = rval;
                    pointerTable[name] = Value(&constants[constantsSize -1]);
                }
                push(pointerTable[name]);
                break;
            }
            case OP_GET_POINTER: {
                Value lval = pop();
                if(lval.type == ValueType::POINTER){
                    push(*lval.as.pointer);
                    break;
                }
                const char* name;
                if(lval.type == ValueType::NUMBER) {
                    name = addNumString(lval.as.number);
                } else if(lval.type == ValueType::STRING){
                    name = lval.as.string;
                } else {
                    runtimeError("Expected pointers only of string or number");
                    return InterpretResult::RUNTIME_ERROR;
                }
                if(pointerTable.find(name) == pointerTable.end()){
                    runtimeError("Undefined variable %s", name);
                    return InterpretResult::RUNTIME_ERROR;
                }
                push(*pointerTable.at(name).as.pointer);
                break;
            }
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





