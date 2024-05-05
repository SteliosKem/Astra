#pragma once
#include "Chunk.h"
#include <vector>
#include <string>
#include "Lexer.h"
#include <unordered_map>
#include "Compiler.h"
#include <time.h>

class CallFrame {
public:
	Function* function;
	int program_counter;
	Value* slots;
	int stack_start_pos;
};

enum Result {
	OK,
	COMPILE_ERROR,
	RUNTIME_ERROR
};

Value time_native(int arg_count, Value* args);
Value input_native(int arg_count, Value* args);

class VM {
public:
	Chunk* chunk;
	std::vector<Value> stack;
	std::vector<CallFrame> frames;

	VM() {
		define_native("time", time_native);
		define_native("input", input_native);
	}

	void free();

	Result interpret(const std::string& input);
	Result compile(const std::string& input, std::string& output);
private:
	Object* objects = nullptr;
	Result binary_operation(ValueType type, TokenType op);
	Result run();
	Value pop_stack();
	Value stack_top();
	void push_stack(Value val) {
		stack.push_back(val);
		CallFrame* frame = &frames[frames.size() - 1];
		frame->slots = &stack[frame->stack_start_pos];
	}
	Value peek(int distance);
	void runtime_error(std::string format);
	bool is_false(Value val) {
		return is_void(val) || (is_bool(val) && !get_bool(val)) || (is_number(val) && !get_number(val));
	}
	bool values_equal(Value a, Value b);
	void concatenate_string();
	void free_objects();
	void free_object(Object* obj);

	bool call_value(Value callee, int arg_count);
	bool call_function(Function* func, int arg_count);
	void define_native(std::string name, NativeFn function);
	std::unordered_map<std::string, Value> globals;
};