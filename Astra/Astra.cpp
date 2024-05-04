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
    bool stop = false;
    bool to_compile = false;                                // Compile or run program
    bool to_run = false;
    bool make_exe = false;                                  // Make ML file or Target Executable
    bool make_ml = false;
    std::string end_path;                                   // Product file path

    std::string source;
    if (argc == 1) {                                        // Repl if no file was specified
        
        while (true) {
            std::cout << "> ";
            std::getline(std::cin, source);

            vm.interpret(source);
        }
    }
    else {
        std::string input;
        for (int i = 1; i < argc; i++) {
            if (argv[i][0] != '-') {                            // Run from file if argument is not a parameter
                std::ifstream file;
                file.open(argv[1]);
                end_path = argv[1];
                if (!file) {                                    // If the file doesn't exist
                    std::cout << "Could not open file " << argv[1] << std::endl;
                    exit(74);
                }

                while (file) {                                  // Read file
                    source += file.get();
                }

                

                for (int i = 0; i < source.size() - 1; i++) {
                    input += source[i];
                }
            }
            else {
                std::string cmd = std::string(argv[i]);
                if (cmd == "-h" || cmd == "--help") {
                    std::cout << "Astra list of commands:" << std::endl <<
                        "-h / --help: Show list of arguments." << std::endl <<
                        "<File path>: Select a file as source string. If this is omitted you will enter into REPL mode." << std::endl <<
                        "-c / --compile: Output a file" << std::endl <<
                        "-r / --run: Execute the source file immediately" << std::endl <<
                        "-e / --make_executable: Output a target executable file, add =<file path> to specify the path and name of the file" << std::endl <<
                        "-m / --make_machine: Output a bytecode file, add =<file path> to specify the path and name of the file" << std::endl;
                    return 0;
                }
                else if (cmd == "-c" || cmd == "--compile") to_compile = true;
                else if (cmd == "-r" || cmd == "--run") to_run = true;
                else if (cmd == "-e" || cmd == "--make_executable") make_exe = true;
                else if (cmd == "-m" || cmd == "--make_machine") make_ml = true;
                else {
                    std::cout << "Unkown command: " << cmd << std::endl;
                    return 0;
                }
            }
        }

        if (make_exe && make_ml) {
            std::cout << "Cannot use make_exe and make_ml at the same time.";
            return 0;
        }
        if (to_run && to_compile) {
            std::cout << "Cannot use run and compile at the same time.";
            return 0;
        }
        if (to_run && (make_exe || make_ml)) {
            std::cout << "Cannot use run and compile at the same time.";
            return 0;
        }

        if (to_run) {
            Result res = vm.interpret(input);

            if (res == Result::COMPILE_ERROR) exit(65);
            if (res == Result::RUNTIME_ERROR) exit(70);
        }
        if (to_compile) {
            if (make_ml) {
                std::string path;
                for (char& i : end_path) {
                    if (i != '.')
                        path += i;
                    else {
                        path += ".ml";
                        break;
                    }
                }
                std::ofstream Output(path);
                std::string out;
                

                Result res = vm.compile(input, out);

                if (res == Result::COMPILE_ERROR) exit(65);
                if (res == Result::RUNTIME_ERROR) exit(70);
                // Write to the file
                Output << out;

                // Close the file
                Output.close();
            }
        }
    }

    vm.free();
}