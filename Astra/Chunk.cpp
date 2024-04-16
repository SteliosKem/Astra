#include "Chunk.h"

#include <iostream>

void Chunk::write(uint8_t byte, int line) {
	code.push_back(byte);
	lines.push_back(line);
}

void Chunk::free() {
	code.clear();
	constants.free();
}

void Chunk::disassemble(const char* name) {
	std::cout << "== " << name << " ==" << std::endl;

	for (int offset = 0; offset < code.size();) {
		offset = disassembleInstruction(offset);
	}
}

int Chunk::disassembleInstruction(int offset) {
	std::cout << offset << " ";
	if (offset > 0 && lines[offset] == lines[offset - 1])
		std::cout << "    | ";
	else {
		std::cout << "    " << lines[offset] << " ";
	}

	uint8_t instruction = code[offset];
	switch (instruction)
	{
	case OpCode::CONSTANT:
		return constantInstruction("CONSTANT", offset);
	case OpCode::NEGATE:
		return simpleInstruction("OP_NEGATE", offset);
	case OpCode::ADD:
		return simpleInstruction("OP_ADD", offset);
	case OpCode::SUBTRACT:
		return simpleInstruction("OP_SUBTRACT", offset);
	case OpCode::MULTIPLY:
		return simpleInstruction("OP_MULTIPLY", offset);
	case OpCode::DIVIDE:
		return simpleInstruction("OP_DIVIDE", offset);
	case OpCode::RETURN:
		return simpleInstruction("RETURN", offset);
	default:
		std::cout << "Unkown OpCode " << instruction;
		return offset + 1;
	}
	
}

int Chunk::simpleInstruction(const char* name, int offset) {
	std::cout << name;
	std::cout << std::endl;
	return offset + 1;
}

int Chunk::addConstant(Value value) {
	constants.write(value);
	return constants.values.size() - 1;
}

int Chunk::constantInstruction(const char* name, int offset) {
	uint8_t constant = code[offset + 1];
	std::cout << name << " " << constant;
	printValue(constants.values[constant]);
	std::cout << std::endl;
	return offset + 2;
}