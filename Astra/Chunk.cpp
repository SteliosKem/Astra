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
	case OC_PUSH:
		return simple_instruction("OC_PUSH", offset);
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
	case OC_POWER:
		return simple_instruction("OP_POWER", offset);
	case OC_CALL:
		return byte_instruction("OP_CALL", offset);
	case OC_GET_UPVALUE:
		return byte_instruction("OP_GET_UPVALUE", offset);
	case OC_SET_UPVALUE:
		return byte_instruction("OP_SET_UPVALUE", offset);
	case OC_CLOSE_UPVALUE:
		return simple_instruction("OP_CLOSE_UPVALUE", offset);
	case OC_CLOSURE: {
		offset++;
		uint8_t constant = code[offset++];
		std::cout << "OP_CLOSURE " << unsigned(constant) << " ";
		print_value(constants.values[constant]);
		std::cout << std::endl;

		Function* function = get_function(constants.values[constant]);
		for (int j = 0; j < function->upvalue_count; j++) {
			int is_local = code[offset++];
			int index = code[offset++];
			std::cout << "          | " << offset - 2 << " " << (is_local ? "local" : "upvalue") << " " << index;
		}

		std::cout << std::endl;

		return offset;
	}
	case OC_CLASS:
		return constant_instruction("OC_CLASS", offset);
	case OC_ENUM:
		return constant_instruction("OC_ENUM", offset);
	case OC_ENUM_VALUE:
		return constant_instruction("OC_ENUM_VALUE", offset);
	case OC_GET_MEMBER:
		return constant_instruction("OC_GET_MEMBER", offset);
	case OC_SET_MEMBER:
		return constant_instruction("OC_SET_MEMBER", offset);
	case OC_GET_INDEX:
		return simple_instruction("OC_GET_INDEX", offset);
	case OC_GET_INDEX_COMPOUND:
		return simple_instruction("OC_GET_INDEX_COMPOUND", offset);
	case OC_SET_INDEX:
		return simple_instruction("OC_SET_INDEX", offset);
	case OC_METHOD:
		return constant_instruction("OC_METHOD", offset);
	case OC_GET_MEMBER_COMPOUND:
		return constant_instruction("OC_GET_MEMBER_COMPOUND", offset);
	case OC_INVOKE:
		return invoke_instruction("OC_INVOKE", offset);
	case OC_INHERIT:
		return simple_instruction("OP_INHERIT", offset);
	case OC_GET_SUPER:
		return constant_instruction("OC_GET_SUPER", offset);
	case OC_SUPER_INVOKE:
		return invoke_instruction("OC_SUPER_INVOKE", offset);
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
	std::cout << name << " " << unsigned(constant) << ": " << " ";
	print_value(constants.values[constant]);
	std::cout << std::endl;
	return offset + 2;
}

int Chunk::byte_instruction(const char* name, int offset) {
	uint8_t slot = code[offset + 1];
	std::cout << name << " " << unsigned(slot) << std::endl;
	return offset + 2;
}

int Chunk::jump_instruction(const char* name, int sign, int offset) {
	uint16_t jump = (uint16_t)(code[offset + 1] << 8);
	jump |= code[offset + 2];
	std::cout << name << " " << offset << " -> " << offset + 3 + sign * jump << std::endl;
	return offset + 3;
}

int Chunk::invoke_instruction(const char* name, int offset) {
	uint8_t constant = code[offset + 1];
	uint8_t arg_count = code[offset + 2];
	std::cout << name << " (" << unsigned(arg_count) << ") " << unsigned(constant) << " ";
	print_value(constants.values[constant]);
	std::cout << std::endl;
	return offset + 3;
}

bool is_function(Value val) {
	return get_object(val)->type == OBJ_FUNCTION;
}

bool is_closure(Value val) {
	return get_object(val)->type == OBJ_CLOSURE;
}

Function* get_function(Value val) {
	return (Function*)get_object(val);
}

Closure* get_closure(Value val) {
	return (Closure*)get_object(val);
}

void print_object(Value value) {
	switch (get_object(value)->type) {
	case OBJ_STRING:
		std::cout << get_string(value)->str;
		break;
	case OBJ_FUNCTION:
	{
		Function* func = get_function(value);
		if (func->name == "") std::cout << "<script>";
		else std::cout << "<function " << func->name << '>';
		break;
	}
	case OBJ_NATIVE:
		std::cout << "<native function>";
		break;
	case OBJ_UPVALUE:
		std::cout << "upvalue";
		break;
	case OBJ_CLOSURE: {
		Function* func = get_closure(value)->function;
		if (func->name == "") std::cout << "<script>";
		else std::cout << "<function " << func->name << '>';
		break;
	}
	case OBJ_CLASS:
		std::cout << "<class " << get_class(value)->name << ">";
		break;
	case OBJ_INSTANCE:
		std::cout << "<" << get_instance(value)->class_target->name << " instance>";
		break;
	case OBJ_ENUM:
		std::cout << "<" << get_enum(value)->name << " enum>";
		break;
	case OBJ_ENUM_VAL:
		std::cout << "<" << get_enum_val(value)->value << " enum value>";
		break;
	case OBJ_LIST: {
		std::cout << '[';
		List* list = get_list(value);
		for (int i = 0; i < list->values.size(); i++) {
			print_value(list->values[i]);
			if (i + 1 < list->values.size()) {
				std::cout << ", ";
			}
		}
		std::cout << ']';
		break;
	}
	case OBJ_BOUND_METHOD: {
		Function* func = get_bound_method(value)->method->function;
		std::cout << "<function " << func->name << '>';
		break;
	}
	}

}

BoundMethod* get_bound_method(Value val) {
	return (BoundMethod*)get_object(val);
}
bool is_bound_method(Value val) {
	return val.type == VALUE_OBJECT && std::get<Object*>(val.value)->type == OBJ_BOUND_METHOD;
}