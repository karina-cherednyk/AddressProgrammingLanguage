#ifndef CHUNK_H
#define CHUNK_H


#include <cstdint>
#include <vector>

enum OpCode {
    OP_RETURN,
    OP_CONSTANT,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_LESS,
    OP_EQUAL,
    OP_GREATER
};
enum class ValueType {
    BOOL,
    NUMBER,
    POINTER
};

typedef uint8_t byte;

struct Value {
    ValueType type;
    union {
        bool boolean;
        double number;
        Value* pointer;
    } as;
    inline explicit Value(double value):type(ValueType::NUMBER), as({.number = value}){};
    inline explicit Value(bool value):type(ValueType::BOOL), as({.boolean = value}){};
    inline explicit Value(Value* value):type(ValueType::POINTER), as({.pointer = value}){};
};

//typedef double Value;

struct Chunk {
    std::vector<byte> code;
    std::vector<int> lines;
    std::vector<Value> constants;

    void write(byte val, int line);
    int addConstant(Value const_val);
};


#endif //CHUNK_H
