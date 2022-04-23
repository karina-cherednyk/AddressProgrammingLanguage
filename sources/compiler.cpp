#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstdarg>
#include "../headers/compiler.h"


const Compiler::ParseFn Compiler::getPrefixFn(TokenType type){
    switch (type) {
        case TokenType::LEFT_PAREN: return &Compiler::grouping;
        case TokenType::NUMBER: return  &Compiler::number;
        case TokenType::FALSE:
        case TokenType::TRUE: return &Compiler::literal;
        case TokenType::MINUS:
        case TokenType::BANG: return &Compiler::unary;
        case TokenType::IDENTIFIER: return  &Compiler::variable;
        case TokenType::SINGLE_QUOTE: return  &Compiler::pointer;
        default: return nullptr;
    }
}

const Compiler::ParseRule Compiler::getInfixRule(TokenType type) {
    switch (type) {
        case TokenType::PLUS:
        case TokenType::MINUS: return {&Compiler::binary, PREC_TERM};
        case TokenType::STAR:
        case TokenType::SLASH: return {&Compiler::binary, PREC_FACTOR};
        case TokenType::LESS:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL:  return {&Compiler::binary, PREC_COMPARISON};
        case TokenType::EQUAL_EQUAL:
        case TokenType::BANG_EQUAL:  return {&Compiler::binary, PREC_EQUALITY};
        case TokenType::EQUAL_GREATER: return {&Compiler::refer, PREC_ASSIGNMENT};
        case TokenType::LESS_EQUAL_GREATER: return {&Compiler::exchange, PREC_ASSIGNMENT};
        default: return {nullptr, PREC_NONE};
    }
}



void Compiler::writeReturn() {
    writeByte(OP_RETURN);
}

void Compiler::endCompiler(){
    writeReturn();
}




void Compiler::parsePrecedence(Precedence precedence, bool advanceFirst) {
    if(advanceFirst) parser.advance(); // first token become parser.previous
    ParseFn prefixFn = getPrefixFn(parser.previous.type);
    if(prefixFn == nullptr){
        parser.errorAt(parser.previous, "Expect prefix operator or literal");
        return;
    }
    (this->*prefixFn)(); // stop on the next operator or at end
    // continue to consume parts with higher precedence
    ParseRule infixRule{};
    while ( infixRule = getInfixRule(parser.current.type), infixRule.precedence >= precedence){
        parser.advance();
        (this->*infixRule.fn)();
    }
}

void Compiler::writeConstant(Value value){
    int constant_position = chunk->addConstant(value);
    assert(constant_position <= UINT8_MAX);
    writeBytes(OP_CONSTANT, (byte) constant_position);
}

void Compiler::writeString(std::string s){
    const char* str = addString(s.c_str(), s.size());
    writeConstant(Value(str));
}


void Compiler::grouping(){
    expression();
    parser.consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
}



void Compiler::binary(){
    TokenType operatorType = parser.previous.type;
    const ParseRule& rule = getInfixRule(operatorType);
    parsePrecedence((Precedence)((int)rule.precedence + 1));
    switch (operatorType) {
        case TokenType::MINUS:
            writeByte(OP_SUBTRACT); break;
        case TokenType::PLUS:
            writeByte(OP_ADD); break;
        case TokenType::STAR:
            writeByte(OP_MULTIPLY); break;
        case TokenType::SLASH:
            writeByte(OP_DIVIDE); break;
        case TokenType::LESS:
            writeByte(OP_LESS); break;
        case TokenType::GREATER:
            writeByte(OP_GREATER); break;
        case TokenType::LESS_EQUAL:
            writeBytes(OP_GREATER, OP_NOT); break;
        case TokenType::GREATER_EQUAL:
            writeBytes(OP_LESS, OP_NOT); break;
        case TokenType::EQUAL_EQUAL:
            writeByte(OP_EQUAL); break;
        case TokenType::BANG_EQUAL:
            writeBytes(OP_EQUAL, OP_NOT); break;
        default:return;
    }
}

void Compiler::unary(){
    TokenType operatorType = parser.previous.type;
    parsePrecedence(PREC_UNARY);

    switch (operatorType) {
        case TokenType::MINUS:
            writeByte(OP_NEGATE); break;
        case TokenType::BANG:
            writeByte(OP_NOT);  break;
        default: return;
    }
}
void Compiler::number() {
    double value = strtod(parser.previous.start, nullptr);
    writeConstant(Value(value));
}

void Compiler::variable() {
    const char* idStr = addString(parser.previous.start, parser.previous.length);
    writeConstant(Value(idStr));
}

void Compiler::literal(){
    bool value = parser.previous.type == TokenType::TRUE;
    writeByte(value ? OP_TRUE : OP_FALSE);
}

void Compiler::expression(bool advanceFirst) {
    parsePrecedence(PREC_ASSIGNMENT, advanceFirst);
}
void Compiler::expressionStatement(bool advanceFirst) {
    expression(advanceFirst);
    writeByte(OP_POP);
}


void Compiler::printStatement() {
    expression();
    writeByte(OP_PRINT);
}

size_t Compiler::writeJump(byte command){
    writeByte(command);
    writeByte(0xff);
    return chunk->count() - 1;
}

void Compiler::patchJump(size_t commandIdx){
    chunk->code[commandIdx] = chunk->count() - commandIdx - 1;
}


void Compiler::ifStatement() {
    parser.consume(TokenType::LEFT_CURLY, "Expected '{' after PR keyword.");
    expression();
    parser.consume(TokenType::RIGHT_CURLY, "Expected '}' after predicate.");

    size_t ifFalseJump = writeJump(OP_JUMP_IF_FALSE);


    if(!parser.currentEqual(4, TokenType::INLINE_DIVIDER, TokenType::NEW_LINE, TokenType::EOF, TokenType::HORIZONTAL)){
        do { statement();
        }while(parser.previous.type == TokenType::INLINE_DIVIDER);
    }
    size_t trueEndJump = writeJump(OP_JUMP);

    patchJump(ifFalseJump);

    if(parser.match(TokenType::HORIZONTAL)){
        do{ statement();
        }while(parser.previous.type == TokenType::INLINE_DIVIDER);
    }
    patchJump(trueEndJump);
    // Stack: FALSE_BRANCH, TRUE_BRANCH, PREDICATE

}

void Compiler::BStatement() {
    while ( parser.match(TokenType::INLINE_DIVIDER) ||
            parser.match(TokenType::NEW_LINE));
    writeByte(OP_PART_END);
}

void Compiler::checkLabel() {
    parser.advance();
    if(parser.previous.type == TokenType::IDENTIFIER && parser.peek(TokenType::DOTS_3))
    {
        std::string  labelName(parser.previous.start, parser.previous.start + parser.previous.length);
        addLabel(labelName);
        parser.advance(); // consume ...
        parser.advance(); // consume next token
    }
}

void Compiler::addLabel(std::string labelName) {
    chunk->labelMap[labelName] = chunk->count();
}

void Compiler::statement() {
    if(parser.previous.type == TokenType::NEW_LINE) checkLabel();
    else parser.advance(); // 'if' will advance one token no no matter if there was a label or not

    switch (parser.previous.type) {
        case TokenType::B: BStatement(); break;
        case TokenType::BANG: writeReturn(); return;
        case TokenType::PR: ifStatement(); break;
        case TokenType::L: loopStatement(); break;
        case TokenType::PRINT: printStatement(); break;
        case TokenType::EOF: return;
        case TokenType::NEW_LINE: break;
        default:
            expressionStatement(false);
    }

    while ( parser.match(TokenType::INLINE_DIVIDER) ||
            parser.match(TokenType::NEW_LINE));

    if(parser.panicMode) parser.synchronize();
}





void Compiler::writeByte(byte byte1) {
    chunk->write(byte1, parser.previous.line);
}

void Compiler::writeBytes(byte byte1, byte byte2) {
    chunk->write(byte1, parser.previous.line);
    chunk->write(byte2, parser.previous.line);

}


void Compiler::pointer() {
    if(parser.match(TokenType::IDENTIFIER)){
        const char* idStr = addString(parser.previous.start, parser.previous.length);
        writeConstant(Value(idStr));
    }
    else {
        parsePrecedence(PREC_UNARY); // evaluate lvalue value
    }
    if(parser.match(TokenType::EQUAL)){ // set
        expression();
        writeByte(OP_SET_POINTER);
    } else { // get
        writeByte(OP_GET_POINTER);
    }

}

void Compiler::refer() {
    parsePrecedence((Precedence)((int)Precedence::PREC_ASSIGNMENT + 1));
    writeByte(OP_SET_POINTER_INVERSE);
}

void Compiler::exchange() {
    parsePrecedence((Precedence)((int)Precedence::PREC_ASSIGNMENT + 1));
    writeByte(OP_EXCHANGE);
}

void Compiler::write(Chunk &tchunk) {
    chunk->write(tchunk);
}
