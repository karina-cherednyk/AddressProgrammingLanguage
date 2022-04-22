#include "../headers/compiler.h"


void Compiler::compileExpression(Chunk* chunk){
    this->chunk = chunk;
    expression();
}
void Compiler::compileConditionExpression(Chunk* chunk){
    this->chunk = chunk;
    parser.consume(TokenType::LEFT_CURLY, "Expected '{' after PR keyword.");
    expression();
    parser.consume(TokenType::RIGHT_CURLY, "Expected '}' after predicate.");
}

bool Compiler::compile(const char*source, Chunk* chunk){
    parser.scanner.init(source);
    this->chunk = chunk;
    parser.hadError = false;
    parser.panicMode = false;


    parser.advance(); // first token become parser.previous
    while(!parser.match(TokenType::EOF)) statement();
    endCompiler();
    return !parser.hadError;
}


void Compiler::compileUntil(std::string label){
    Value* lastval = nullptr;
    do {
        statement();
        if(!chunk->constants.empty())
            lastval = &chunk->constants.at(chunk->constants.size() - 1);
    }while(lastval == nullptr || lastval->type != ValueType::STRING || lastval->val.string != label);
}

void Compiler::loopStatement() {
    Compiler innerComp{parser};
    Chunk initialization, step, end, endCondition, parameter, l1, l2;

    parser.consume(TokenType::LEFT_CURLY, "Expected '{' before loop declaration.");

    //initialization part
    innerComp.compileExpression( &initialization);

    // step part
    parser.consume(TokenType::LEFT_PAREN, "Expected '('.");
    innerComp.compileExpression( &step);
    parser.consume(TokenType::RIGHT_PAREN, "Expected ')'.");

    // condition part
    bool isCondExpression = parser.match(TokenType::PR);
    if(isCondExpression){
        innerComp.compileConditionExpression(&end);
        endCondition = end;
    } else innerComp.compileExpression(&end);

    //parameter part
    parser.consume(TokenType::DOT, "Expected '.'.");
    parser.consume(TokenType::EQUAL_GREATER, "Expected '=>'.");
    innerComp.compileExpression( &parameter);
    if(!isCondExpression){ // patch the condition
        endCondition.write(parameter);
        endCondition.write(OP_GET_POINTER, parser.current.line);
        endCondition.write(end);
        endCondition.write(OP_EQUAL, parser.current.line);
        endCondition.write(OP_NOT, parser.current.line);
    }


    parser.consume(TokenType::RIGHT_CURLY, "Expected '}' after loop declaration.");
    innerComp.compileExpression( &l1);
    parser.consume(TokenType::INLINE_DIVIDER, "Expected ','");
    innerComp.compileExpression( &l2);
    Value cl1 = l1.constants[0];
    //init
    write(initialization);
    write(parameter);
    writeByte(OP_SET_POINTER_INVERSE);
    writeByte(OP_POP);
    size_t skipFirstIncrementJump = writeJump(OP_JUMP);
    //begin body
    //
    addLabel(cl1.val.string);
    // increment
    write(parameter);
    write(parameter);
    writeByte(OP_GET_POINTER);
    write(step);
    writeByte(OP_ADD);
    writeByte(OP_SET_POINTER);

    patchJump(skipFirstIncrementJump);
    // condition
    write(endCondition);
    write(l2);
    writeByte(OP_JUMP_IF_FALSE_TO_LABEL);

    compileUntil(cl1.val.string);

}
