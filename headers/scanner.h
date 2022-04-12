#ifndef SCANNER_H
#define SCANNER_H

enum class TokenType {
// Single-character tokens.
    COMMA, SEMICOLON,
    MINUS, PLUS, SLASH, STAR,
// One or two character tokens.
    BANG, BANG_EQUAL, EQUAL, EQUAL_EQUAL, GREATER,
    GREATER_EQUAL, LESS, LESS_EQUAL,  SINGLE_QUOTE, EQUAL_GREATER,
// Literals.
    IDENTIFIER,  NUMBER, ERROR, EOF
// Keywords.
/*        TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE, TOKEN_FOR, TOKEN_FUN, TOKEN_IF,
        TOKEN_NIL, TOKEN_OR, TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS, TOKEN_TRUE,
        TOKEN_VAR, TOKEN_WHILE, ERROR, TOKEN_EOF*/
};

struct Token {
    TokenType type;
    const char* start;
    int length;
    int line;
};

class Scanner {


    const char* start;
    const char* current;
    int line;

    Token makeToken(TokenType type);
    char advance();
    bool match(char expected);
    char peek();
    char peekNext();
    void skipWhitespaces();
    static bool isDigit(char c);
    static bool isAlpha(char c);
    Token number();
    TokenType identifierType();
    Token identifier();


public:
    Scanner(const char* source){
        start = source;
        current = source;
        line = 1;
    }
    bool isAtEnd();

    Token errorToken(const char* message);
    Token scanToken();
};


#endif //SCANNER_H
