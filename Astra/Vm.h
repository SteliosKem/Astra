#pragma once
#include "Chunk.h"
#include <stack>

enum Result {
	OK,
	COMPILE_ERROR,
	RUNTIME_ERROR
};

class VM {
public:
	Chunk* chunk;
	std::stack<Value> stack;

	void free();
	Result interpret(Chunk* _chunk);
private:
	uint8_t* program_counter;
	Result run();
	Value pop_stack();
};