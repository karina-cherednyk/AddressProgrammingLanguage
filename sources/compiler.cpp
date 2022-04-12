#include "../headers/compiler.h"
Compiler::Compiler():
    rules { [(int)TokenType::LEFT_PAREN] = {&Compiler::unary, NULL, PREC_NONE},
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
              [(int)TokenType::TRUE] = {NULL, NULL, PREC_NONE},
              [(int)TokenType::FALSE] = {NULL, NULL, PREC_NONE},

              }
{}

