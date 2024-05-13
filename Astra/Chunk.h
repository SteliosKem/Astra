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
	OC_POWER,
	OC_NEGATE,
	OC_RETURN,
	OC_NOT,
	OC_EQUAL,
	OC_LESS,
	OC_GREATER,
	OC_PRINT,
	OC_POP,
	OC_DEFINE_GLOBAL,
	OC_GET_GLOBAL,
	OC_SET_GLOBAL,
	OC_GET_LOCAL,
	OC_SET_LOCAL,
	OC_JMP_IF_FALSE,
	OC_JMP,
	OC_LOOP,
	OC_CALL,
	OC_CLOSURE,
	OC_SET_UPVALUE,
	OC_GET_UPVALUE,
	OC_CLOSE_UPVALUE
};

class Chunk {
public:
	std::vector<uint8_t> code;										// Bytes
	ValueArray constants;											// Constant Pool
	std::vector<int> lines;

	void write(uint8_t byte, int line);								// Write byte to chunk
	void free();
	void disassemble(std::string name);
	int add_constant(Value value);									// Add to constant pool
	
private:
	int disassemble_instruction(int offset);
	int constant_instruction(const char* name, int offset);
	static int simple_instruction(const char* name, int offset);
	int byte_instruction(const char* name, int offset);
	int jump_instruction(const char* name, int sign, int offset);
};

class Function : public Object {
public:
	int arity = 0;
	Chunk chunk;
	std::string name;
	int upvalue_count = 0;
};

class UpvalueObj : public Object {
public:
	UpvalueObj(Value* loc) {
		location = loc;
		type = OBJ_UPVALUE;
	}
	Value* location;
	Value closed = make_void();
	UpvalueObj* next = nullptr;
};

class Closure : public Object {
public:
	Closure(Function* func) {
		function = func;
		upvalue_count = func->upvalue_count;
		type = OBJ_CLOSURE;
	}
	Closure() {
		type = OBJ_CLOSURE;
	}
	Function* function;
	std::vector<UpvalueObj*> upvalues;
	int upvalue_count;
};

bool is_function(Value val);
bool is_closure(Value val);
Function* get_function(Value val);
Closure* get_closure(Value val);