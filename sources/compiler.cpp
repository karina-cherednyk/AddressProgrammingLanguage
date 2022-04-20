#include <cstdio>
#include <cstdlib>
#include <cassert>
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
        default: return NULL;
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
        default: return {NULL, PREC_NONE};
    }
}



void Compiler::emitReturn() {
    writeByte(OP_RETURN);
}

void Compiler::endCompiler(){
    emitReturn();
}

void Compiler::Parser::errorAt(Token& token, const char *message) {
    if(panicMode) return;
    fprintf(stderr, "[line %d] Error", token.line);

    switch (token.type) {
        case TokenType::EOF: fprintf(stderr, " at end"); break;
        case TokenType::ERROR: break;
        default: fprintf(stderr, " at '%.*s'", token.length, token.start);
    }

    fprintf(stderr, ": %s\n", message);
    hadError = true;
}

void Compiler::Parser::errorAtCurrent(const char *errMsg) {
    errorAt(current,errMsg );
}

bool Compiler::Parser::match(TokenType type){
    if(current.type != type) return false;
    advance();
    return true;
}


void Compiler::Parser::advance(){
    previous = current;
    while(true){
        current = scanner.scanToken();
        if(current.type != TokenType::ERROR) break;
        errorAtCurrent(current.start);
    }
}
void Compiler::Parser::consume(TokenType type, const char* errMsg){
    if(current.type == type) advance();
    else errorAtCurrent(errMsg);
}

void Compiler::Parser::synchronize() {
    panicMode = false;
    while (current.type != TokenType::EOF){
        if(previous.type == TokenType::DIVIDER) return;
        switch (current.type) {
            case TokenType::PRINT:
                return;
            default: ;// do nothing
        }
        advance();
    }
}


void Compiler::parsePrecedence(Precedence precedence) {
    parser.advance(); // first token become parser.previous
    ParseFn prefixFn = getPrefixFn(parser.previous.type);
    if(prefixFn == NULL){
        parser.errorAt(parser.previous, "Expect prefix operator or literal");
        return;
    }
    (this->*prefixFn)(); // stop on the next operator or at end
    // continue to consume parts with higher precedence
    ParseRule infixRule;
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

void Compiler::expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}
void Compiler::expressionStatement() {
    expression();
    while(parser.match(TokenType::DIVIDER)) ;
    writeByte(OP_POP);
}


void Compiler::printStatement() {
    expression();
    while(parser.match(TokenType::DIVIDER)) ;
    writeByte(OP_PRINT);
}




void Compiler::statement() {
    if(parser.match(TokenType::PRINT)){
        printStatement();
    }
    else {
        expressionStatement();
    }
    if(parser.panicMode) parser.synchronize();
}



bool Compiler::compile(const char*source, Chunk* chunk){
#ifdef DEBUG_H
    disassembleInstructions(chunk);
#endif

    parser.scanner.init(source);
    this->chunk = chunk;
    parser.hadError = false;
    parser.panicMode = false;


    parser.advance(); // first token become parser.previous
    while(!parser.match(TokenType::EOF)) statement();
    endCompiler();
    return !parser.hadError;
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

