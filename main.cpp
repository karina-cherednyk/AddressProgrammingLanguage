#include <iostream>
#include <fstream>
#include <sstream>
#include "headers/vm.h"

Vm vm;

static void runFile(const char* path){
    std::ifstream  in(path);
    std::stringstream  buffer;
    buffer << in.rdbuf();
    std::string s = buffer.str();
    const char* source = s.c_str();
    in.close();
    InterpretResult result = vm.interpret(source);
    if (result == InterpretResult::COMPILE_ERROR) exit(65);
    if(result == InterpretResult::RUNTIME_ERROR) exit(70);
}

static void repl() {
    char line[1024];
    while (true){
        printf("> ");

        if(!fgets(line, sizeof (line), stdin)){
            printf("\n");
            break;
        }
        vm.interpret(line);
    }

}

int main(int argc, const char* argv[]) {
    vm.initVM();
//    if(argc == 1) repl();
//    else if(argc == 2) runFile(argv[1]);
    runFile("..\\unitTests\\test3.txt");
    vm.freeVM();

    return 0;
}
