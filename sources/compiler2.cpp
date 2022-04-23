#include <cstdarg>
#include "../headers/compiler.h"
using namespace std;

void Compiler::compileExpression(Chunk* chunk){
    this->chunk = chunk;
    parsePrecedence( (Precedence)(Precedence::PREC_ASSIGNMENT + 1));
}
void Compiler::compileConditionExpression(Chunk* chunk){
    this->chunk = chunk;
    parser.consume(TokenType::LEFT_CURLY, "Expected '{' after PR keyword.");
    expression();
    parser.consume(TokenType::RIGHT_CURLY, "Expected '}' after predicate.");
}

bool Compiler::compile(const char*source, Chunk* chunk){
    parser.scanner.init(source);
    this->chunk = chunk;
    parser.hadError = false;
    parser.panicMode = false;


    parser.advance(); // first token become parser.previous
    while(!parser.match(TokenType::EOF)) statement();
    endCompiler();
    return !parser.hadError;
}


void Compiler::compileUntil(std::string label){
    Value* lastval = nullptr;
    do {
        statement();
        if(!chunk->constants.empty())
            lastval = &chunk->constants.at(chunk->constants.size() - 1);
    }while(lastval == nullptr || lastval->type != ValueType::STRING || lastval->val.string != label);
}
int Compiler::ForLoopParts::initLabel = 0;

void Compiler::parseForLoopParts(ForLoopParts* parts){
    Chunk end;
    //initialization part
    compileExpression( &parts->initialization);

    // step part
    parser.consume(TokenType::LEFT_PAREN, "Expected '('.");
    compileExpression( &parts->step);
    parser.consume(TokenType::RIGHT_PAREN, "Expected ')'.");

    // condition part
    bool isCondExpression = parser.match(TokenType::PR);
    if(isCondExpression){
        compileConditionExpression(&end);
        parts->endCondition = end;
    } else compileExpression(&end);

    //parameter part
    if(!parser.match(TokenType::EQUAL_GREATER)){
        parts->nextPart = new ForLoopParts;
        parser.consume(TokenType::INLINE_DIVIDER, "Expected either '=>' or ','.");
        parseForLoopParts(parts->nextPart);
        parts->parameter = parts->nextPart->parameter;
    }
    else {
        compileExpression( &parts->parameter);
    }

    if(!isCondExpression){ // patch the condition
        parts->endCondition.write(parts->parameter);
        parts->endCondition.write(OP_GET_POINTER, parser.current.line);
        parts->endCondition.write(end);
        parts->endCondition.write(OP_LESS, parser.current.line);
    }
}

#include "../headers//utility.h"

void Compiler::writeInitPart(const vector<ForLoopParts *> &forLoopParts, string l1, int forLoopNumber) {
    for(int i=0; i< forLoopParts.size(); i++){
        auto forLoop = forLoopParts.at(i);
        int j=0;
        do{
            addLabel( format("_init_%d_%d.%d", forLoopNumber, i, j));
            write(forLoop->parameter);
            write(forLoop->initialization);
            writeByte(OP_SET_POINTER_WITHOUT_PUSH);

            writeString(format("_cond_%d_%d", forLoopNumber, i));
            writeString(format("_cond_%d_%d.%d", forLoopNumber, i, j));
            writeByte(OP_GET_LABEL);
            writeByte(OP_SET_POINTER_WITHOUT_PUSH);

            if(i == 0) writeString(l1);
            else       writeString(format("_incr_%d_%d", forLoopNumber, i));
            writeString(format("_incr_%d_%d.%d", forLoopNumber, i, j));
            writeByte(OP_GET_LABEL);
            writeByte(OP_SET_POINTER_WITHOUT_PUSH);

            if(j == 0 && i == forLoopParts.size() - 1 ){
                writeString(format("_cond_%d_%d", forLoopNumber, 0)); // jump to cond begin, skip incr end
                writeByte(OP_POP);
            }
            else if(j == 0){
                writeString(format("_init_%d_%d.%d", forLoopNumber, i+1,0)); // jump to init next sequence
                writeByte(OP_POP);
            }
            else {
                writeString(format("_cond_%d_%d", forLoopNumber, i+1)); // jump to next condition or condition end
                writeByte(OP_POP);
            }
        } while (j++, forLoop = forLoop->nextPart);
    }
}

void Compiler::writeIncrementPart(const std::vector<ForLoopParts*>& forLoopParts, int forLoopNumber){
    for(int i=0; i< forLoopParts.size(); i++) {
        auto forLoop = forLoopParts.at(i);
        int j = 0;
        do {
            addLabel(format("_incr_%d_%d.%d", forLoopNumber, i, j));
            // *parameter = *parameter + step
            write(forLoop->parameter);
            write(forLoop->parameter);
            writeByte(OP_GET_POINTER);
            write(forLoop->step);
            writeByte(OP_ADD);
            writeByte(OP_SET_POINTER_WITHOUT_PUSH);
            // jump from loopNum_x to loopNum_(x+1)
            if(i!= forLoopParts.size() -1)
                writeString(format("_incr_%d_%d", forLoopNumber, i+1));
            else
                writeString(format("_cond_%d_%d", forLoopNumber, 0));
            writeByte(OP_POP);
        }while (j++, forLoop = forLoop->nextPart);
    }
}

void Compiler::writeConditionPart(const std::vector<ForLoopParts*>& forLoopParts, std::string  l2, int forLoopNumber){
    for(int i=0; i< forLoopParts.size(); i++) {
        auto forLoop = forLoopParts.at(i);
        int j = 0;
        do{
            addLabel(format("_cond_%d_%d.%d", forLoopNumber, i, j));
            write(forLoop->endCondition);
            // if false jump to init_0_i_(j+1) or l2
            if(forLoop->nextPart == nullptr) writeString(l2);
            else writeString(format("_init_%d_%d.%d", forLoopNumber, i, j+1));
            writeByte(OP_JUMP_IF_FALSE_TO_LABEL);

            // else jump to cond 0_(i+1)
            writeString(format("_cond_%d_%d", forLoopNumber, i+1)); // jump to next condition or condition end
            writeByte(OP_POP);
        }while (j++, forLoop = forLoop->nextPart);
    }
    addLabel(format("_cond_%d_%d", forLoopNumber, forLoopParts.size()));
}

void Compiler::loopStatement() {
    static int LoopNUMBER = 0;
    Compiler innerComp{parser};
    Chunk l1, l2;
    vector<ForLoopParts*> forLoops;
    parser.consume(TokenType::LEFT_CURLY, "Expected '{' before loop declaration.");
    do   {
        auto* n = new ForLoopParts;
        innerComp.parseForLoopParts(n);
        forLoops.push_back(n);
    }
    while(parser.match(TokenType::INLINE_DIVIDER));


    parser.consume(TokenType::RIGHT_CURLY, "Expected '}' after loop declaration.");
    innerComp.compileExpression( &l1);
    parser.consume(TokenType::INLINE_DIVIDER, "Expected ','");
    innerComp.compileExpression( &l2);
    Value cl1 = l1.constants[0];
    Value cl2 = l2.constants[0];
    if(cl1.type != ValueType::STRING || cl2.type != ValueType::STRING)
    {
        parser.errorAtCurrent("Expected l1, l2 labels for 'for loop'");
    }

    writeInitPart(forLoops, cl1.val.string, LoopNUMBER);
    writeIncrementPart(forLoops, LoopNUMBER);
    writeConditionPart(forLoops, cl2.val.string, LoopNUMBER);

    for(auto & forLoop : forLoops) delete forLoop;
    compileUntil(cl1.val.string);
    LoopNUMBER++;
}
