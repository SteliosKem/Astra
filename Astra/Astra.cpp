#include <iostream>
#include "Chunk.h"
#define DEBUG_TRACE_EXECUTION
#include "Vm.h"



int main()
{
    VM vm;
    Chunk chunk;
    int constant = chunk.addConstant(1.2);
    chunk.write(OpCode::CONSTANT, 0);
    chunk.write(constant, 0);

    chunk.write(OpCode::RETURN, 0);
    chunk.disassemble("test");

    vm.interpret(&chunk);

    vm.free();
    chunk.free();
}