#include "Vm.h"
#include "Compiler.h"
#include <iostream>

Result VM::interpret(Chunk* _chunk) {
	chunk = _chunk;
	program_counter = 0;
	return run();
}

Result VM::run() {
	while (true) {
#ifdef DEBUG_TRACE_EXECUTION
		std::cout << "        ";
		if (!stack.empty()) {
			for (Value* slot = &(stack.top()) - stack.size(); slot < &(stack.top()); slot++) {
				std::cout << "[ ";
				print_value(*slot);
				std::cout << " ]";
			}
		}
		std::cout << std::endl;
		chunk->disassemble_instruction((int)(program_counter - &(chunk->code[0])));
#endif
		uint8_t instruction;
		Value b;
		switch (instruction = chunk->code[program_counter++]) {
		case OpCode::RETURN:
			print_value(pop_stack());
			
			std::cout << std::endl;
			return Result::OK;
		case OpCode::ADD:
			b = pop_stack();
			stack.push(pop_stack() + b);
			break;
		case OpCode::SUBTRACT:
			b = pop_stack();
			stack.push(pop_stack() - b);
			std::cout << "hi";
			break;
		case OpCode::MULTIPLY: 
			b = pop_stack();
			stack.push(pop_stack() * b);
			break;
		case OpCode::DIVIDE: 
			b = pop_stack();
			stack.push(pop_stack() / b);
			break;
		case OpCode::NEGATE:
			stack.push(-pop_stack());
			break;
		case OpCode::CONSTANT:
			Value constant = chunk->constants.values[chunk->code[program_counter++]];
			stack.push(constant);
			break;
		}
		
	}
}

void VM::free() {

}

Value VM::pop_stack() {
	Value current_val = stack.top();
	stack.pop();
	return current_val;
}

Result VM::interpret(const std::string& input) {
	Parser parser{};
	Compiler compiler(input, parser);
	Chunk new_chunk;
	chunk = &new_chunk;
	if (!compiler.compile(chunk)) {
		chunk->free();
		return COMPILE_ERROR;
	}
	program_counter = 0;
	/*for (int i = 0; i < chunk->code.size(); i++) {
		std::cout << (int)(chunk->code[i]) << std::endl;
	}*/
	Result result = run();

	chunk->free();
	return result;
}