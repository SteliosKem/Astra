#pragma once
#include "Chunk.h"
#include <vector>
#include <string>
#include "Lexer.h"
#include <unordered_map>
#include "Compiler.h"

struct CallFrame {
	Function* function;
	int program_counter;
	Value* slots;
};

enum Result {
	OK,
	COMPILE_ERROR,
	RUNTIME_ERROR
};

class VM {
public:
	CallFrame frames[64];
	int frame_count;
	Chunk* chunk;
	std::vector<Value> stack;

	void free();
	Result interpret(Chunk* _chunk);
	Result interpret(const std::string& input);
private:
	int program_counter;
	Object* objects = nullptr;
	Result binary_operation(ValueType type, TokenType op);
	Result run();
	Value pop_stack();
	Value stack_top();
	Value peek(int distance);
	void runtime_error(const std::string& format, ...);
	bool is_false(Value val) {
		return is_void(val) || (is_bool(val) && !get_bool(val)) || (is_number(val) && !get_number(val));
	}
	bool values_equal(Value a, Value b);
	void concatenate_string();
	void free_objects();
	void free_object(Object* obj);
	
	std::unordered_map<std::string, Value> globals;
};