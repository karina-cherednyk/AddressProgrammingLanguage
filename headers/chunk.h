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
    OP_GREATER,
    OP_TRUE,
    OP_FALSE,
    OP_PRINT,
    OP_POP,
    OP_DEFINE_VAR,
    OP_GET_VAR,
    OP_SET_VAR
};
enum class ValueType {
    BOOL,
    NUMBER,
    POINTER,
    STRING
};

typedef uint8_t byte;

struct Value {
    ValueType type;
    union {
        bool boolean;
        double number;
        const char* string;
        Value* pointer;
    } as;
    inline explicit Value(double value):type(ValueType::NUMBER), as({.number = value}){};
    inline explicit Value(bool value):type(ValueType::BOOL), as({.boolean = value}){};
    inline explicit Value(Value* value):type(ValueType::POINTER), as({.pointer = value}){};
    inline explicit Value(const char* value):type(ValueType::STRING), as({.string = value}){};
    inline Value():type(ValueType::BOOL), as({.boolean = false}){};

    bool operator== (const Value& other) const;
    void printValue();
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
