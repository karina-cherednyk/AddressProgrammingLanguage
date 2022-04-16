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
        if(previous.type == TokenType::SEMICOLON) return;
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
    double value = strtod(parser.previous.start, NULL);
    writeConstant(Value(value));
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
    parser.consume(TokenType::SEMICOLON, "Expect ';' after value.");
    writeByte(OP_POP);
}


void Compiler::printStatement() {
    expression();
    parser.consume(TokenType::SEMICOLON, "Expect ';' after value.");
    writeByte(OP_PRINT);
}


void Compiler::varDeclaration() {
    byte global = parseVariable("Expect variable name.");
    if(parser.match(TokenType::EQUAL)) expression();
    else writeByte(OP_NIL);
    parser.consume(TokenType::SEMICOLON, "Expect ';' after value.");
    defineVariale(global);
}

void Compiler::statement() {
    if(parser.match(TokenType::PRINT)){
        printStatement();
    }
    else {
        expressionStatement();
    }
}

void Compiler::declaration(){
    if(parser.match(TokenType::VAR)) varDeclaration();
    else statement();
    if(parser.panicMode) parser.synchronize();
}

bool Compiler::compile(const char*source, Chunk* chunk){
    parser.scanner.init(source);
    this->chunk = chunk;
    parser.hadError = false;
    parser.panicMode = false;


    parser.advance(); // first token become parser.previous
    while(!parser.match(TokenType::EOF)) declaration();
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

byte Compiler::parseVariable(const char *errMsg) {
    parser.consume(TokenType::IDENTIFIER, errMsg);
    return chunk->addConstant(Value(parser.previous.start));
}

void Compiler::defineVariable(byte global) {
    writeBytes(OP_DEFINE_VAR, global);
}

void Compiler::variable() {
    Token varName = parser.previous;
    byte nameConst = chunk->addConstant(Value(varName.start));
    if(parser.match(TokenType::EQUAL)){
     expression();
        writeBytes(OP_SET_VAR, nameConst);
    }
    else {
        writeBytes(OP_GET_VAR, nameConst);
    }
}


