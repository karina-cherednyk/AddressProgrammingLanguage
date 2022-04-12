#ifndef COMPILER_H
#define COMPILER_H


#include "scanner.h"
#include "chunk.h"



class Compiler {
    Compiler();

    struct Parser {
        Token current;
        Token previous;
        bool hadError;
        bool panicMode;
    };



    Parser parser;
    void binary();
    void grouping();
    void unary();
    void number();




    /*
     * expression -> assignment
     * assignment -> address_assignment | value_assignment
     * address_assignment -> ??
     * value_assignment = (IDENTIFIER '=' assignment) | or
     * or = and ( 'or' and )*
     * and = eq ( 'and' eq )*
     * eq = comp (  ('=='|'!=') comp )*
     * comp = term ( ('>'|'>='|'<'|'<=') term )*
     * term = factor ( ('+'|'-') factor )*
     * factor = unary ( '*'|'/' unary )*
     * unary = ('-'|'!') primary
     * primary = 'true' | 'false' | NUMBER | IDENTIFIER | '(' expression ')'
     * */
    enum Precedence {
        PREC_NONE, PREC_ASSIGNMENT, PREC_OR, PREC_AND,
        PREC_EQUALITY, PREC_COMPARISON, PREC_TERM, PREC_FACTOR,
        PREC_UNARY, PREC_PRIMARY
    };

    typedef void (Compiler::*ParseFn)();
    struct ParseRule {
        ParseFn prefix;
        ParseFn infix;
        Precedence precedence;
    };
    //ParseFn getParseRule(TokenType type);
    ParseRule rules[];


public:
    bool compile(const char* source, Chunk* chunk);

};


#endif //COMPILER_H
