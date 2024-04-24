#include <iostream>
#include "Chunk.h"
#define DEBUG_TRACE_EXECUTION
#define DEBUG_PRINT_CODE
#include "Vm.h"
#include <string>
#include <fstream>


int main(int argc, const char* argv[])
{
    VM vm;
    std::string source;
    if (argc == 1) {                                        // Repl if no file was specified
        
        while (true) {
            std::cout << "> ";
            std::getline(std::cin, source);

            vm.interpret(source);
        }
    }
    else {
        if (argv[1][0] != '-') {                            // Run from file if the first command is not a parameter
            std::ifstream file;
            file.open(argv[1]);

            if (!file) {                                    // If the file doesn't exist
                std::cout << "Could not open file " << argv[1] << std::endl;
                exit(74);
            }

            while (file) {                                  // Read file
                source += file.get();
            }

            Result res = vm.interpret(source);

            if (res == Result::COMPILE_ERROR) exit(65);
            if (res == Result::RUNTIME_ERROR) exit(70);
        }
    }
    
    Chunk chunk;
    /*int constant = chunk.add_constant(1.2);
    chunk.write(OpCode::CONSTANT, 0);
    chunk.write(constant, 0);

    chunk.write(OpCode::RETURN, 0);
    chunk.disassemble("test");*/

    vm.interpret(&chunk);

    vm.free();
}