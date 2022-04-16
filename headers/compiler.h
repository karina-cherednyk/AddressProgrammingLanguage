#ifndef COMPILER_H
#define COMPILER_H


#include "scanner.h"
#include "chunk.h"


class Compiler {

    enum Precedence {
        PREC_NONE, PREC_ASSIGNMENT, PREC_OR, PREC_AND,
        PREC_EQUALITY, PREC_COMPARISON, PREC_TERM, PREC_FACTOR,
        PREC_UNARY, PREC_PRIMARY
    };

    typedef void (Compiler::*ParseFn)();
    struct ParseRule {
        ParseFn fn;
        Precedence precedence;
    };

    /*
     * declaration -> varDecl | statement
     * statement -> exprStatement | printStatement
     * */
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


    struct Parser {
        Token current;
        Token previous;
        bool hadError;
        bool panicMode;
        Scanner scanner;

        void errorAtCurrent(const char *errMsg);
        void errorAt(Token& token, const char *errMsg);
        void advance();
        bool match(TokenType type);
        void consume(TokenType type,  const char* errMsg);

        void synchronize();
    };



    Parser parser;
    Chunk* chunk;

    void writeByte(byte byte1);
    void writeBytes(byte byte1, byte byte2);
    void writeConstant(Value value);
    void emitReturn();


    byte parseVariable(const char* errMsg);
    void defineVariable(byte num);

    void declaration();
    void varDeclaration();
    void statement();
    void printStatement();
    void expressionStatement();
    void parsePrecedence(Precedence precedence);
    void expression();
    void endCompiler();


    void binary();
    void grouping();
    void unary();
    void number();
    void literal();
    void variable();

    //
    //ParseRule rules[50];
    static const ParseFn getPrefixFn(TokenType type);
    static const ParseRule getInfixRule(TokenType type);

public:
    bool compile(const char* source, Chunk* chunk);
};


#endif //COMPILER_H
