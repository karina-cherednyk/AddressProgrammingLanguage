#include "../headers/scanner.h"
#include "cstring"
#include "assert.h"


Scanner::Scanner(const char* source){
    start = source;
    current = source;
    line = 1;
}

bool Scanner::isAtEnd(){
    *current == '\0';
}
Token Scanner::makeToken(TokenType type){
    Token t;
    t.type = type;
    if(type != TokenType::IDENTIFIER && type != TokenType::NUMBER) return t;
    t.start = start;
    t.length = (int)(current - start);
    t.line = line;
    return t;
}

Token Scanner::errorToken(const char* message){
    Token t;
    t.type = TokenType::ERROR;
    t.start = message;
    t.length = strlen(message);
    t.line = line;
    return t;
}

char Scanner::advance(){
    assert(*current != '\0');
    return *current++;
}

bool Scanner::match(char expected){
    if(*current == expected) {
        advance();
        return true;
    }
    return false;
}

char Scanner::peek(){
    return *current;
}

char Scanner::peekNext(){
    if(isAtEnd()) return  '\0';
    return *(current + 1);
}

void Scanner::skipWhitespaces(){
    while (true){
        char  c = peek();
        switch (c) {
            case ' ':
            case '\t':
            case '\r':
                advance(); break;
            default: return;
        }
    }
}

bool Scanner::isDigit(char c){
    return c >= '0' && c <= '9';
}

bool Scanner::isAlpha(char c) {
    return  c >= 'a' && c <= 'z' ||
            c >= 'A' && c <= 'Z' ||
            c == '_';
}

Token Scanner::number(){
    while (isDigit(peek())) advance();
    if(peek() == '.' && isDigit(peekNext())) {
        advance();
        while (isDigit(peek())) advance();
    }
    return makeToken(TokenType::NUMBER);
};

TokenType Scanner::checkKeyword(const char* with, TokenType type){
    size_t length = strlen(with);
    if( (size_t)(this->current - this->start) == length && memcmp(this->start , with, length) == 0)
        return type;
    else return TokenType::IDENTIFIER;
}
TokenType Scanner::identifierType(){
    //TODO: check for keywords.
    switch (*start) {
        case 't':
            return checkKeyword("true", TokenType::TRUE);
        case 'f':
            return checkKeyword("false", TokenType::FALSE);
    }
    return TokenType::IDENTIFIER;
}

Token Scanner::identifier() {
    while(isAlpha(peek()) || isDigit(peek())) advance();
    return makeToken(identifierType());
}

Token Scanner::scanToken() {
    skipWhitespaces();
    start = current;
    if(isAtEnd()) return makeToken(TokenType::EOF);
    char c = advance();
    if(isDigit(c)) return number();
    if(isAlpha(c)) return identifier();
    switch (c) {
        case ';': return makeToken(TokenType::SEMICOLON);
        case ',': return makeToken(TokenType::COMMA);
        case '-': return makeToken(TokenType::MINUS);
        case '+': return makeToken(TokenType::PLUS);
        case '*': return makeToken(TokenType::STAR);
        case '/': return makeToken(TokenType::SLASH);
        case '\'': return makeToken(TokenType::SINGLE_QUOTE);
        case '(': return makeToken(TokenType::LEFT_PAREN);
        case ')': return makeToken(TokenType::RIGHT_PAREN);
        case '!': return makeToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);

        case '<': return makeToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
        case '>': return makeToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
        case '=': {
            if(match('=')) return makeToken(TokenType::EQUAL_EQUAL);
            if(match('>')) return makeToken(TokenType::EQUAL_GREATER);
            else return makeToken(TokenType::EQUAL);
        }
    }
    return errorToken("Unexped token");
}