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
    OP_DIVIDE
};
typedef uint8_t byte;
typedef double Value;

struct Chunk {
    std::vector<byte> code;
    std::vector<int> lines;
    std::vector<Value> constants;

    void write(byte val, int line);
    int addConstant(Value const_val);
};


#endif //CHUNK_H
