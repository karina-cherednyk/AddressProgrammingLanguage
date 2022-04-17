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
Value* Vm::addToMemory(const Value& value){
    memory[memorySize++] = value;
    return &memory[memorySize-1];
}
template <typename T, typename U>
bool has(const std::map<T,U>& map, T val){
    return map.find(val) != map.end();
}

InterpretResult Vm::setPointer(bool inverse){
    Value pointee, pointer;
    if(inverse) pointer = pop(), pointee = pop();
    else pointee = pop(), pointer = pop();

    const char* pointerName;
    if(pointer.type == ValueType::NUMBER) {
        pointerName = addNumString(pointer.as.number);
    } else if(pointer.type == ValueType::STRING){
        pointerName = pointer.as.string;
    }
    Value& mapPointer = pointer.type == ValueType::MAP_POINTER ? pointer : pMap[pointerName];

    if(pointee.type == ValueType::STRING) {
        const char* rname =  pointee.as.string;
        if(!has(pMap, rname)) pMap[rname].as.pointTo = addToMemory(Value());
        mapPointer.as.pointTo->as.pointTo = pMap[rname].as.pointTo;
    } else if(pointee.type == ValueType::NUMBER){
        mapPointer.as.pointTo->as.pointTo = addToMemory(pointee);
    } else if(pointee.type == ValueType::MAP_POINTER){
        mapPointer.as.pointTo->as.pointTo = pointee.as.pointTo;
    } else
        assert(false);

    push(mapPointer);
    return InterpretResult::OK;
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
                pop().printValue();  printf("\n"); break;
            case OP_POP:
                pop(); break;

            case OP_SET_POINTER:
                if(setPointer(false) == InterpretResult::RUNTIME_ERROR) return  InterpretResult::RUNTIME_ERROR;
            case OP_GET_POINTER: {
                Value lval = pop();
                const char* name;
                if(lval.type == ValueType::MAP_POINTER) { push(Value::MapPointer(lval.as.pointTo->as.pointTo)); break; }
                else if(lval.type == ValueType::STRING) name = lval.as.string;
                else  name = addNumString(lval.as.number);

                if (has(pMap, name)) {
                    push(pMap[name]);
                }
                else { runtimeError("Undefined pointTo %s", name); return InterpretResult::RUNTIME_ERROR; }
                break;
            }
            case OP_CONSTANT:
                push(chunk->constants[readByte()]); break;
            case OP_TRUE:
                push(Value(0.0)); break;
            case OP_FALSE:
                push(Value(false)); break;
            case OP_NEGATE:
                CHECK_NEXT_NUMBER(0);
                push(Value(-(pop()).as.number));
                break;
            case OP_NOT:
                push(Value(isFalsey(pop()) ? 0.0 : 1.0)  ); break;
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
            case OP_GREATER:
                BINARY_OP(>); break;
            case OP_EQUAL:
            {
                Value b = pop();
                Value a = pop();
                push(Value(a == b));
            }
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
    return value.type != ValueType::NUMBER  ||  value.as.number != 0 ;
}





