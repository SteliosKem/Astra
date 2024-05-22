#pragma once
#include "Chunk.h"
#include <vector>
#include <string>
#include "Lexer.h"
#include <unordered_map>
#include "Compiler.h"
#include <time.h>

#define FRAMES_MAX 64
#define STACK_MAX FRAMES_MAX * UINT8_MAX

class CallFrame {
public:
	Closure* closure;
	//int program_counter;
	uint8_t* program_counter;
	Value* slots;
	//int stack_start_pos;
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
	//std::vector<Value> stack;
	//std::vector<CallFrame> frames;
	Value stack[STACK_MAX];
	Value* stack_top;

	CallFrame frames[FRAMES_MAX];
	int frame_count;

	VM() {
		stack_top = stack;
		define_native("time", time_native);
		define_native("input", input_native);
	}

	void free();

	Compiler* compiler;

	Result interpret(const std::string& input);
	Result compile(const std::string& input, std::string& output);

	std::unordered_map<std::string, Value> globals;
	UpvalueObj* open_upvalues = nullptr;
private:
	Object* objects = nullptr;
	
	Result binary_operation(ValueType type, TokenType op);
	Result run();
	Value pop_stack() {
		stack_top--;
		return *stack_top;
	}
	//Value stack_top();
	/*void push_stack(Value val) {
		stack.push_back(val);
		CallFrame* frame = &frames[frames.size() - 1];
		frame->slots = &stack[frame->stack_start_pos];
	}*/
	Value push_stack(Value val) {
		*stack_top = val;
		return *stack_top++;
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

	UpvalueObj* capture_upvalue(Value* local);
	void close_upvalues(Value* last);

	std::vector<Object*> gray_stack;

	bool call_value(Value callee, int arg_count);
	bool call_function(Closure* closure, int arg_count);
	void define_native(std::string name, NativeFn function);
	void collect_garbage();
	void mark_roots();

	void define_method(std::string name);
	bool bind_method(ClassObj* _class, std::string name);
	bool invoke(std::string name, int arg_count);
	bool invoke_from_class(ClassObj* _class, std::string name, int arg_count);
};

