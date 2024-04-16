#pragma once
#include <vector>
#include "Value.h"

enum OpCode {
	CONSTANT,
	ADD,
	SUBTRACT,
	MULTIPLY,
	DIVIDE,
	NEGATE,
	RETURN,
};

class Chunk {
public:
	std::vector<uint8_t> code;
	ValueArray constants;
	std::vector<int> lines;

	void write(uint8_t byte, int line);
	void free();
	void disassemble(const char* name);
	int addConstant(Value value);
	int constantInstruction(const char* name, int offset);
	int disassembleInstruction(int offset);
private:
	
	static int simpleInstruction(const char* name, int offset);
};