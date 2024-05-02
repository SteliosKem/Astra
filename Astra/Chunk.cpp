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

void Chunk::disassemble(std::string name) {
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
	case OC_CONSTANT:
		return constant_instruction("CONSTANT", offset);
	case OC_NEGATE:
		return simple_instruction("OP_NEGATE", offset);
	case OC_NOT:
		return simple_instruction("OP_NOT", offset);
	case OC_VOID:
		return simple_instruction("OP_VOID", offset);
	case OC_TRUE:
		return simple_instruction("OP_TRUE", offset);
	case OC_FALSE:
		return simple_instruction("OP_FALSE", offset);
	case OC_ADD:
		return simple_instruction("OP_ADD", offset);
	case OC_SUBTRACT:
		return simple_instruction("OP_SUBTRACT", offset);
	case OC_MULTIPLY:
		return simple_instruction("OP_MULTIPLY", offset);
	case OC_DIVIDE:
		return simple_instruction("OP_DIVIDE", offset);
	case OC_RETURN:
		return simple_instruction("RETURN", offset);
	case OC_EQUAL:
		return simple_instruction("OP_EQUAL", offset);
	case OC_GREATER:
		return simple_instruction("OP_GREATER", offset);
	case OC_LESS:
		return simple_instruction("OP_LESS", offset);
	case OC_PRINT:
		return simple_instruction("OP_PRINT", offset);
	case OC_POP:
		return simple_instruction("OP_POP", offset);
	case OC_DEFINE_GLOBAL:
		return constant_instruction("OP_DEFINE_GLOBAL", offset);
	case OC_GET_GLOBAL:
		return constant_instruction("OP_GET_GLOBAL", offset);
	case OC_SET_GLOBAL:
		return constant_instruction("OP_SET_GLOBAL", offset);
	case OC_GET_LOCAL:
		return byte_instruction("OP_GET_LOCAL", offset);
	case OC_SET_LOCAL:
		return byte_instruction("OP_SET_LOCAL", offset);
	case OC_JMP:
		return jump_instruction("OP_JMP", 1, offset);
	case OC_JMP_IF_FALSE:
		return jump_instruction("OP_JMP_IF_FALSE", 1, offset);
	case OC_LOOP:
		return jump_instruction("OP_LOOP", -1, offset);
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

int Chunk::byte_instruction(const char* name, int offset) {
	uint8_t slot = code[offset + 1];
	std::cout << name << " " << slot;
	return offset + 2;
}

int Chunk::jump_instruction(const char* name, int sign, int offset) {
	uint16_t jump = (uint16_t)(code[offset + 1] << 8);
	jump |= code[offset + 2];
	std::cout << name << " " << offset << " -> " << offset + 3 + sign * jump;
	return offset + 3;
}