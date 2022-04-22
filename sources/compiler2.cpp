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

Compiler::ForLoopParts Compiler::parseForLoopParts(){
    ForLoopParts parts;
    Chunk end;
    //initialization part
    compileExpression( &parts.initialization);

    // step part
    parser.consume(TokenType::LEFT_PAREN, "Expected '('.");
    compileExpression( &parts.step);
    parser.consume(TokenType::RIGHT_PAREN, "Expected ')'.");

    // condition part
    bool isCondExpression = parser.match(TokenType::PR);
    if(isCondExpression){
        compileConditionExpression(&end);
        parts.endCondition = end;
    } else compileExpression(&end);

    //parameter part
    parser.consume(TokenType::DOT, "Expected '.'.");
    parser.consume(TokenType::EQUAL_GREATER, "Expected '=>'.");
    compileExpression( &parts.parameter);
    if(!isCondExpression){ // patch the condition
        parts.endCondition.write(parts.parameter);
        parts.endCondition.write(OP_GET_POINTER, parser.current.line);
        parts.endCondition.write(end);
        parts.endCondition.write(OP_EQUAL, parser.current.line);
        parts.endCondition.write(OP_NOT, parser.current.line);
    }
    return parts;
}

void Compiler::loopStatement() {
    Compiler innerComp{parser};
    Chunk l1, l2;
    std::vector<ForLoopParts> forLoops;
    parser.consume(TokenType::LEFT_CURLY, "Expected '{' before loop declaration.");
    do   forLoops.push_back(innerComp.parseForLoopParts());
    while(parser.match(TokenType::INLINE_DIVIDER));


    parser.consume(TokenType::RIGHT_CURLY, "Expected '}' after loop declaration.");
    innerComp.compileExpression( &l1);
    parser.consume(TokenType::INLINE_DIVIDER, "Expected ','");
    innerComp.compileExpression( &l2);
    Value cl1 = l1.constants[0];


    // code part
    //init
    for(auto it = forLoops.begin(); it != forLoops.end(); it++) {
        write(it->initialization);
        write(it->parameter);
        writeByte(OP_SET_POINTER_INVERSE);
        writeByte(OP_POP);
    }

    size_t skipFirstIncrementJump = writeJump(OP_JUMP);
    //begin body
    //
    addLabel(cl1.val.string);
    // increment
    for(auto it = forLoops.begin(); it != forLoops.end(); it++) {
        write(it->parameter);
        write(it->parameter);
        writeByte(OP_GET_POINTER);
        write(it->step);
        writeByte(OP_ADD);
        writeByte(OP_SET_POINTER);
    }

    patchJump(skipFirstIncrementJump);
    // condition
    for(auto it = forLoops.begin(); it != forLoops.end(); it++) {
        write(it->endCondition);
        write(l2);
        writeByte(OP_JUMP_IF_FALSE_TO_LABEL);
    }

    compileUntil(cl1.val.string);

}
