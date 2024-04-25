#pragma once
#include <vector>
#include "Value.h"

enum OpCode {
	OC_CONSTANT,
	OC_VOID,
	OC_TRUE,
	OC_FALSE,
	OC_ADD,
	OC_SUBTRACT,
	OC_MULTIPLY,
	OC_DIVIDE,
	OC_NEGATE,
	OC_RETURN,
	OC_NOT,
	OC_EQUAL,
	OC_LESS,
	OC_GREATER
};

class Chunk {
public:
	std::vector<uint8_t> code;										// Bytes
	ValueArray constants;											// Constant Pool
	std::vector<int> lines;

	void write(uint8_t byte, int line);								// Write byte to chunk
	void free();
	void disassemble(const char* name);
	int add_constant(Value value);									// Add to constant pool
	
private:
	int disassemble_instruction(int offset);
	int constant_instruction(const char* name, int offset);
	static int simple_instruction(const char* name, int offset);
};