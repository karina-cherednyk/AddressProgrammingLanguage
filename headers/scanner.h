#ifndef SCANNER_H
#define SCANNER_H
#undef EOF

enum  class TokenType {
// Single-character tokens.
    NEW_LINE, INLINE_DIVIDER,
    MINUS, PLUS, SLASH, STAR, LEFT_PAREN, RIGHT_PAREN, B, LEFT_CURLY, RIGHT_CURLY, HORIZONTAL,
// One or two character tokens.
    BANG, BANG_EQUAL, EQUAL, EQUAL_EQUAL, GREATER,
    GREATER_EQUAL, LESS, LESS_EQUAL,  SINGLE_QUOTE, EQUAL_GREATER,
// Three characters
    DOTS_3,
// Literals.
    IDENTIFIER,  NUMBER, ERROR, EOF,
// Keywords.
    TRUE, FALSE, PRINT, PR,
/*        TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE, TOKEN_FOR, TOKEN_FUN, TOKEN_IF,
        TOKEN_NIL, TOKEN_OR, TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS, TOKEN_TRUE,
        TOKEN_VAR, TOKEN_WHILE, ERROR, TOKEN_EOF*/

};

struct Token {
    TokenType type;
    const char* start{nullptr};
    int length{0};
    int line{-1};
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
    TokenType checkKeyword(const char* with, TokenType type);
    TokenType identifierType();
    Token identifier();


public:
    Scanner();
    void init(const char* source);
    bool isAtEnd();

    Token errorToken(const char* message);
    Token scanToken();
};


#endif //SCANNER_H
