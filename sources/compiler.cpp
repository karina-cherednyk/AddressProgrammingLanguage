#include <cstdio>
#include <cstdlib>
#include <cassert>
#include "../headers/compiler.h"
Compiler::Compiler():
    rules { [(int)TokenType::LEFT_PAREN] = {&Compiler::grouping, NULL, PREC_NONE},
              [(int)TokenType::RIGHT_PAREN] = {NULL, NULL, PREC_NONE},
              [(int)TokenType::COMMA] = {NULL, NULL, PREC_NONE},
              [(int)TokenType::MINUS] = {&Compiler::unary, &Compiler::binary, PREC_TERM},
              [(int)TokenType::PLUS] = {NULL, &Compiler::binary, PREC_TERM},
              [(int)TokenType::SEMICOLON] = {NULL, NULL, PREC_NONE},
              [(int)TokenType::SLASH] = {NULL, &Compiler::binary, PREC_FACTOR},
              [(int)TokenType::STAR] = {NULL, &Compiler::binary, PREC_FACTOR},
              [(int)TokenType::BANG] = {NULL, NULL, PREC_NONE},
              [(int)TokenType::BANG_EQUAL] = {NULL, NULL, PREC_NONE},
              [(int)TokenType::EQUAL] = {NULL, NULL, PREC_NONE},
              [(int)TokenType::EQUAL_EQUAL] = {NULL, NULL, PREC_NONE},
              [(int)TokenType::GREATER] = {NULL, NULL, PREC_NONE},
              [(int)TokenType::GREATER_EQUAL] = {NULL, NULL, PREC_NONE},
              [(int)TokenType::LESS] = {NULL, NULL, PREC_NONE},
              [(int)TokenType::LESS_EQUAL] = {NULL, NULL, PREC_NONE},
              [(int)TokenType::SINGLE_QUOTE] = {NULL, NULL, PREC_NONE}, // TODO
              [(int)TokenType::EQUAL_GREATER] = {NULL, NULL, PREC_NONE}, // TODO
              [(int)TokenType::IDENTIFIER] = {NULL, NULL, PREC_NONE},
              [(int)TokenType::NUMBER] = {&Compiler::number, NULL, PREC_NONE},
              [(int)TokenType::ERROR] = {NULL, NULL, PREC_NONE},
              [(int)TokenType::EOF] = {NULL, NULL, PREC_NONE},
              [(int)TokenType::TRUE] = {&Compiler::literal, NULL, PREC_NONE},
              [(int)TokenType::FALSE] = {&Compiler::literal, NULL, PREC_NONE},

              }
{}

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

void Compiler::Parser::advance(){
    previous = current;
    while(true){
        current = scanner.scanToken();
        if(current.type != TokenType::ERROR) break;
        errorAtCurrent(current.start);
    }
}
const Compiler::ParseRule& Compiler::getRule(TokenType type){
    return rules[(int )type];
}


void Compiler::parsePrecedence(Precedence precedence) {
    parser.advance(); // first token become parser.previous
    ParseFn prefixRule = getRule(parser.previous.type).prefix;
    if(prefixRule == NULL){
        parser.errorAt(parser.previous, "Expect prefix rule or literal");
        return;
    }
    (this->*prefixRule)(); // stop on the next operator or at end
    // continue to consume parts with higher precedence
    while (getRule(parser.current.type).precedence >= precedence){
        parser.advance();
        ParseFn infixRule = getRule(parser.previous.type).infix;
        (this->*infixRule)();
    }
}

void Compiler::writeConstant(Value value){
    int constant_position = chunk->addConstant(value);
    assert(constant_position <= UINT8_MAX);
    writeBytes(OP_CONSTANT, (byte) constant_position);
}


void Compiler::grouping(){
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::binary(){
    TokenType operatorType = parser.previous.type;
    const ParseRule& rule = getRule(operatorType);
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
    writeConstant(Value(value));
}

void Compiler::expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

bool Compiler::compile(const char*source, Chunk* chunk){
    parser.scanner.init(source);
    this->chunk = chunk;
    parser.hadError = false;
    parser.panicMode = false;


    parser.advance(); // first token become parser.previous
    expression();
    consume(TokenType::EOF, "Expect end of expression");

    return !parser.hadError;
}
