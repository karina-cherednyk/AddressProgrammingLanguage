#include <cstdarg>
#include <cstdio>
#include <cassert>
#include "../headers/vm.h"
#include "../headers/debug.h"

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
    programFinished = true;
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

Value* Vm::stringToPointer(std::string s){
    if(has(pMap, s)) return pMap[s];
    else return nullptr;
}

//#undef DEBUG_H
InterpretResult Vm::run(size_t startOffset, size_t endOffset) {
#define CHECK_NEXT_NUMBER(pos) \
    if(peek(pos).type != ValueType::NUMBER){   \
        runtimeError("Expected number.");      \
        return InterpretResult::RUNTIME_ERROR; \
    }                      \

#define BINARY_OP(op) \
    do {                         \
                CHECK_NEXT_NUMBER(0);        \
                CHECK_NEXT_NUMBER(1);        \
                double b = pop().val.number;  \
                double a = pop().val.number;  \
                push(Value(a op b));         \
    } while(false)

#ifdef DEBUG_H
    disassembleInstructions(chunk);
#endif

    for(ip = chunk->code.begin() + startOffset; ip != chunk->code.end() - endOffset && !programFinished; ){

        switch (*ip = readByte()) {
            case OP_RETURN:
                programFinished = true;
                return InterpretResult::OK;
            case OP_PRINT:
            {
                Value v =  pop();
                if(v.type == ValueType::STRING && has(pMap, v.val.string))
                    pMap[v.val.string]->printValue();
                else v.printValue();  printf("\n"); break;
            }
            case OP_JUMP_IF_FALSE:
            {
                byte skipNext =  readByte();
                if(isFalsey(pop())) ip += skipNext;
                break;
            }
            case OP_JUMP: {
                byte skipNext =  readByte();
                ip += skipNext;
                break;
            }
            case OP_EXCHANGE: {
                Value a = pop();
                Value b = pop();
                if(a.type == ValueType::STRING){
                    Value* t = stringToPointer(a.val.string);
                    if(t) a = *t;
                }
                if(b.type == ValueType::STRING){
                    Value* t = stringToPointer(b.val.string);
                    if(t) b = *t;
                }
                if(a.type != ValueType::POINTER || b.type != ValueType::POINTER)
                {
                    runtimeError("Expected 2 pointers to exchange their values");
                    return InterpretResult::RUNTIME_ERROR;
                }
                Value temp = *b.val.pointTo;
                *b.val.pointTo = *a.val.pointTo;
                *a.val.pointTo = temp;
                push(b);
                break;
            }
            case OP_POP:
            {
                Value v = pop();
                std::string label;
                if(v.type == ValueType::NUMBER) label = std::to_string(v.val.number);
                else if(v.type == ValueType::STRING) label = v.val.string;
                else break;
                if(!has(chunk->labelMap, label) ) break;
                run(chunk->labelMap[label]); return InterpretResult::OK;
            }
            case OP_SET_POINTER:
                if(setPointer(false) == InterpretResult::RUNTIME_ERROR)
                    return InterpretResult::RUNTIME_ERROR; break;
            case OP_GET_POINTER:
                if(getPointer() == InterpretResult::RUNTIME_ERROR)
                    return InterpretResult::RUNTIME_ERROR; break;
            case OP_SET_POINTER_INVERSE:
                if(setPointer(true) == InterpretResult::RUNTIME_ERROR)
                    return InterpretResult::RUNTIME_ERROR; break;
            case OP_CONSTANT:
                push(chunk->constants[readByte()]); break;
            case OP_TRUE:
                push(Value(true)); break;
            case OP_FALSE:
                push(Value(false)); break;
            case OP_NEGATE:
                CHECK_NEXT_NUMBER(0);
                push(Value(-(pop()).val.number));
                break;
            case OP_NOT:
                push(Value(isFalsey(pop()))  ); break;
            case OP_ADD:
            {
                Value a = pop();
                Value b = pop();
                if(a.type == ValueType::STRING){
                    Value* t = stringToPointer(a.val.string);
                    if(t) a = *t;
                }
                if(b.type == ValueType::STRING){
                    Value* t = stringToPointer(b.val.string);
                    if(t) b = *t;
                }
                if(a.type == ValueType::NUMBER && b.type == ValueType::NUMBER){
                    push(Value(a.val.number + b.val.number));
                }
                else if(a.type == ValueType::POINTER && b.type == ValueType::NUMBER){
                    push(Value(a.val.pointTo + (int)b.val.number));
                } else if(a.type == ValueType::NUMBER && b.type == ValueType::POINTER){
                    Value* t = (Value* )b.val.pointTo + (int)a.val.number;
                    push(Value(t));
                }
                break;
            }
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
                break;
            }
            case OP_PART_END: return InterpretResult::OK;
        }
    }
    runtimeError("No return statement");
    return InterpretResult::RUNTIME_ERROR;
}
InterpretResult Vm::getPointer(){
    Value pointer = pop();
    const char* name;
    if(pointer.type == ValueType::POINTER) { push(*pointer.val.pointTo); return InterpretResult::OK; }
    else if(pointer.type == ValueType::STRING) name = pointer.val.string;
    else  name = addNumString(pointer.val.number);

    if (has(pMap, name)) {
        push(*pMap[name]->val.pointTo);
    }
    else { runtimeError("Undefined pointTo %s", name); return InterpretResult::RUNTIME_ERROR; }
    return InterpretResult::OK;
}

InterpretResult Vm::setPointer(bool inverse){
    Value pointee, pointer;
    if(inverse) pointer = pop(), pointee = pop();
    else pointee = pop(), pointer = pop();

    const char* pointerName;
    if(pointer.type == ValueType::NUMBER) {
        pointerName = addNumString(pointer.val.number);
    } else if(pointer.type == ValueType::STRING){
        pointerName = pointer.val.string;
    }

    Value* actualPointer;
    if(pointer.type == ValueType::POINTER) actualPointer = &pointer;
    else {
        if(!has(pMap, pointerName)) {
            actualPointer = addToMemory(Value());
            pMap[pointerName] = actualPointer;
        }
        else actualPointer = pMap[pointerName];
    }


    if(pointee.type == ValueType::STRING) {
        const char* rname =  pointee.val.string;
        if(!has(pMap, rname)) return InterpretResult::RUNTIME_ERROR;
        else actualPointer->val.pointTo = pMap[rname];
    } else if(pointee.type == ValueType::NUMBER){
        if(actualPointer->val.pointTo == nullptr) actualPointer->val.pointTo = addToMemory(pointee);
        else *actualPointer->val.pointTo = Value(pointee.val.number);
    } else if(pointee.type == ValueType::BOXED){
        actualPointer->val.pointTo = pointee.val.pointTo;
    } else
        assert(false);

    push(pointee);
    return InterpretResult::OK;
}


InterpretResult Vm::interpret(const char *source) {
    Chunk codeChunk;
    if(!compiler.compile(source, &codeChunk)) return InterpretResult::COMPILE_ERROR;
    this->chunk = &codeChunk;
    programFinished = false;
    InterpretResult result = run();
    this->chunk = NULL;
    return result;
}

bool Vm::isFalsey(Value value) {
    return value.type == ValueType::NUMBER  && value.val.number != 0 ||
           value.type == ValueType::BOOL  && !value.val.boolean;
}





