#ifndef CHUNK_H
#define CHUNK_H


#include <cstdint>
#include <vector>
#include <string>
#include <map>

template <typename T>
bool has(const std::map<std::string ,T>& map, std::string val){
    return map.find(val) != map.end();
}


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
    OP_POP, //a.k.a OP_JUMP_TO_LABEL
    OP_SET_POINTER,
    OP_SET_POINTER_WITHOUT_PUSH,
    OP_GET_POINTER,
    OP_SET_POINTER_INVERSE,
    OP_PART_END,
    OP_JUMP_IF_FALSE,
    OP_JUMP,
    OP_EXCHANGE,
    OP_JUMP_IF_FALSE_TO_LABEL,
    OP_GET_LABEL
};
enum class ValueType {
    NUMBER,
    POINTER,
    STRING,
    BOXED,
    BOOL
};

typedef uint8_t byte;


#include "cstring"


struct Value {
    ValueType type;
    union {
        bool boolean;
        double number;
        const char* string;
        Value* pointTo;

    } val;
    inline explicit Value(bool value): type(ValueType::BOOL), val({.boolean = value}){};
    inline explicit Value(double value): type(ValueType::NUMBER), val({.number = value}){};
    inline explicit Value(Value* value): type(ValueType::POINTER), val({.pointTo = value}){};
    inline explicit Value(const char* value): type(ValueType::STRING), val({.string = value}){};
    inline static Value Boxed(Value* pointee){
        Value p(pointee);
        p.type = ValueType::BOXED;
        return p;
    }
    inline explicit Value():type{ValueType::POINTER}, val({.pointTo = nullptr}) {};
    bool operator== (const Value& other) const;
    void printValue() const;
    explicit operator std::string() const;
};

//typedef double Value;


const char* addString(const char* start, int length);
const char* addString(const char* start);
const char* addNumString(double value);
void freeStrings();

struct Chunk {
    std::vector<byte> code;
    std::vector<int> lines;
    std::vector<Value> constants;

    void write(byte val, int line);
    void write(Chunk& chunk);
    int addConstant(Value const_val);
    inline size_t count(){ return  code.size(); }
    std::map<std::string, size_t> labelMap;
};


#endif //CHUNK_H
