#include "../headers/chunk.h"

void Chunk::write(byte val, int line) {
    lines.push_back(line);
    code.push_back(val);
}


int Chunk::addConstant(Value const_val) {
    constants.push_back(const_val);
    return constants.size() - 1;
}
