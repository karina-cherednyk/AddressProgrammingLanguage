#ifndef COMPILER_H
#define COMPILER_H


#include <map>
#include <string>
#include "scanner.h"
#include "chunk.h"



class Compiler {
public:
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
        bool peek(TokenType type) const;
        void consume(TokenType type,  const char* errMsg);

        void synchronize();

        bool currentEqual(int num, ...) const;
        bool previousEqual(int num, ...) const;
    };

    inline Compiler(Parser& p): parser(p){}

private:

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
     * statement -> exprStatement | printStatement
     * */


    Parser& parser;
    Chunk* chunk;

    void writeByte(byte byte1);
    void writeBytes(byte byte1, byte byte2);
    void write(Chunk& chunk);
    void writeConstant(Value value);
    void writeReturn();
    size_t writeJump(byte command);
    void patchJump(size_t commandIdx);




    void statement();
    void printStatement();
    void BStatement();
    void ifStatement();
    void loopStatement();

    void expressionStatement(bool advanceFirst);
    void parsePrecedence(Precedence precedence, bool advanceFirst = true);
    void expression(bool advanceFirst = true);
    void endCompiler();
    void checkLabel();
    void addLabel(std::string labelName);

    void binary();
    void grouping();
    void unary();
    void number();
    void literal();
    void variable();
    void pointer();
    void refer();
    void exchange();


    //
    //ParseRule rules[50];
    static const ParseFn getPrefixFn(TokenType type);
    static const ParseRule getInfixRule(TokenType type);



public:
    bool compile(const char* source, Chunk* chunk);
    void compileExpression(Chunk* chunk);
    void compileConditionExpression(Chunk* chunk);
    void compileUntil(std::string label);

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



};


#endif //COMPILER_H
