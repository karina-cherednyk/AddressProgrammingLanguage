#include <cstdarg>
#include "../headers/compiler.h"

bool Compiler::Parser::currentEqual(int num, ...) const{
    va_list valist;
    va_start(valist, num);
    for(int i=0; i<num; i++){
        TokenType a = va_arg(valist, TokenType);
        if( a == current.type) {
            va_end(valist);
            return true;
        }
    }
    va_end(valist);
    return false;
}
bool Compiler::Parser::previousEqual(int num, ...) const{
    va_list valist;
    va_start(valist, num);
    for(int i=0; i<num; i++){
        TokenType a = va_arg(valist, TokenType);
        if( a == previous.type) {
            va_end(valist);
            return true;
        }
    }
    va_end(valist);
    return false;
}

void Compiler::Parser::errorAt(Token& token, const char *message) {
    if(panicMode) return;
    fprintf(stderr, "[line %d] Compiler error", token.line);

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
    if(type == TokenType::EOF) return true;
    advance();
    return true;
}
bool Compiler::Parser::peek(TokenType type) const{
    return current.type == type;
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
        if(previousEqual(2, TokenType::INLINE_DIVIDER, TokenType::NEW_LINE)) return;
        switch (current.type) {
            case TokenType::PRINT:
                return;
            default: ;// do nothing
        }
        advance();
    }
}