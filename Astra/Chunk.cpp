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
		offset = disassemble_instruction(offset);
	}
}

int Chunk::disassemble_instruction(int offset) {
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
		return constant_instruction("CONSTANT", offset);
	case OpCode::NEGATE:
		return simple_instruction("OP_NEGATE", offset);
	case OpCode::ADD:
		return simple_instruction("OP_ADD", offset);
	case OpCode::SUBTRACT:
		return simple_instruction("OP_SUBTRACT", offset);
	case OpCode::MULTIPLY:
		return simple_instruction("OP_MULTIPLY", offset);
	case OpCode::DIVIDE:
		return simple_instruction("OP_DIVIDE", offset);
	case OpCode::RETURN:
		return simple_instruction("RETURN", offset);
	default:
		std::cout << "Unkown OpCode " << instruction;
		return offset + 1;
	}
	
}

int Chunk::simple_instruction(const char* name, int offset) {
	std::cout << name;
	std::cout << std::endl;
	return offset + 1;
}

int Chunk::add_constant(Value value) {
	constants.write(value);
	return constants.values.size() - 1;
}

int Chunk::constant_instruction(const char* name, int offset) {
	uint8_t constant = code[offset + 1];
	std::cout << name << " " << constant;
	print_value(constants.values[constant]);
	std::cout << std::endl;
	return offset + 2;
}