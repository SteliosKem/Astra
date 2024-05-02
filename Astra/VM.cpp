#include "Vm.h"

#include <iostream>
#include <cstdarg>
#include <format>


Result VM::interpret(Chunk* _chunk) {
	chunk = _chunk;
	program_counter = 0;
	return run();
}

Result VM::binary_operation(ValueType type, TokenType op) {
	do {
		if (!is_number(peek(0)) || !is_number(peek(1))) {
			runtime_error("Operands must be numbers");
			return RUNTIME_ERROR;
		}
		double b = get_number(pop_stack());
		double a = get_number(pop_stack());

		switch (op) {
		case TOKEN_MINUS:
			stack.push_back(make_number(a - b));
			break;
		case TOKEN_PLUS:
			stack.push_back(make_number(a + b));
			break;
		case TOKEN_STAR:
			stack.push_back(make_number(a * b));
			break;
		case TOKEN_SLASH:
			stack.push_back(make_number(a / b));
			break;
		case TOKEN_GREATER:
			stack.push_back(make_bool(a > b));
			break;
		case TOKEN_LESS:
			stack.push_back(make_bool(a < b));
			break;
		}
		
	} while (false);
	return OK;
}

Result VM::run() {
	CallFrame* frame = &frames[frame_count - 1];

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
		Result res = OK;
		switch (instruction = frame->function->chunk.code[frame->program_counter++]) {
		case OC_RETURN:
			return Result::OK;
		case OC_ADD:
		{
			if (is_string(peek(0)) && is_string(peek(1))) concatenate_string();
			else if (is_number(peek(0)) && is_number(peek(1))) res = binary_operation(VALUE_NUMBER, TOKEN_PLUS);
			else {
				runtime_error("Cannot operate on these types");
				return RUNTIME_ERROR;
			}
			break;
		}
		case OC_SUBTRACT:
			res = binary_operation(VALUE_NUMBER, TOKEN_MINUS);
			break;
		case OC_MULTIPLY:
			res = binary_operation(VALUE_NUMBER, TOKEN_STAR);
			break;
		case OC_DIVIDE:
			res = binary_operation(VALUE_NUMBER, TOKEN_SLASH);
			break;
		case OC_EQUAL:
		{
			Value b = pop_stack();
			Value a = pop_stack();
			stack.push_back(make_bool(values_equal(a, b)));
			break;
		}
		case OC_GREATER:
			res = binary_operation(VALUE_BOOL, TOKEN_GREATER);
			break;
		case OC_LESS:
			res = binary_operation(VALUE_BOOL, TOKEN_LESS);
			break;
		case OC_NOT:
			stack.push_back(make_bool(is_false(pop_stack())));
			break;
		case OC_NEGATE:
			if (!is_number(peek(0))) {
				runtime_error("Operand must be a number");
				return RUNTIME_ERROR;
			}
			stack.push_back(make_number(-get_number(pop_stack())));
			break;
		case OC_CONSTANT:
		{
			Value constant = frame->function->chunk.constants.values[frame->function->chunk.code[frame->program_counter++]];
			stack.push_back(constant);
			break;
		}
		case OC_VOID:
			stack.push_back(make_void());
			break;
		case OC_TRUE:
			stack.push_back(make_bool(true));
			break;
		case OC_FALSE:
			stack.push_back(make_bool(false));
			break;
		case OC_PRINT:
			print_value(pop_stack());
			std::cout << std::endl;
			break;
		case OC_POP:
			pop_stack();
			break;
		case OC_DEFINE_GLOBAL:
		{
			String* name = get_string(frame->function->chunk.constants.values[frame->function->chunk.code[frame->program_counter++]]);
			if (globals.find(name->str) != globals.end()) {
				runtime_error(std::format("Defined already existing variable '{0}'", name->str));
				return RUNTIME_ERROR;
			}
			globals[name->str] = peek(0);
			pop_stack();
			break;
		}
		case OC_GET_GLOBAL:
		{
			String* name = get_string(frame->function->chunk.constants.values[frame->function->chunk.code[frame->program_counter++]]);
			if (globals.find(name->str) == globals.end()) {
				runtime_error(std::format("Undefined variable '{0}'", name->str));
				return RUNTIME_ERROR;
			}
			stack.push_back(globals[name->str]);
			break;
		}
		case OC_SET_GLOBAL:
		{
			String* name = get_string(frame->function->chunk.constants.values[frame->function->chunk.code[frame->program_counter++]]);
			if (globals.find(name->str) == globals.end()) {
				runtime_error(std::format("Undefined variable '{0}'", name->str));
				return RUNTIME_ERROR;
			}
			globals[name->str] = peek(0);
			break;
		}
		case OC_GET_LOCAL:
		{
			uint8_t slot = frame->function->chunk.code[frame->program_counter++];
			stack.push_back(frame->slots[slot]);
			break;
		}
		case OC_SET_LOCAL:
		{
			uint8_t slot = frame->function->chunk.code[frame->program_counter++];
			frame->slots[slot] = peek(0);
			break;
		}
		case OC_JMP_IF_FALSE:
		{
			frame->program_counter += 2;
			uint16_t offset = (uint16_t)(frame->function->chunk.code[frame->program_counter - 2] << 8 | frame->function->chunk.code[frame->program_counter - 1]);
			if (is_false(peek(0))) frame->program_counter += offset;
			break;
		}
		case OC_JMP:
		{
			frame->program_counter += 2;
			uint16_t offset = (uint16_t)(frame->function->chunk.code[frame->program_counter - 2] << 8 | frame->function->chunk.code[frame->program_counter - 1]);
			frame->program_counter += offset;
			break;
		}
		case OC_LOOP:
		{
			frame->program_counter += 2;
			uint16_t offset = (uint16_t)(frame->function->chunk.code[frame->program_counter - 2] << 8 | frame->function->chunk.code[frame->program_counter - 1]);
			frame->program_counter -= offset;
			break;
		}
		}
		
		if (res == RUNTIME_ERROR) {
			return RUNTIME_ERROR;
		}
	}
}

void VM::free() {
	free_objects();
}

void VM::free_objects() {
	Object* obj = objects;
	while(obj != nullptr) {
		Object* next = obj->next;
		free_object(obj);
		obj = next;
	}
}

void VM::free_object(Object* obj) {
	delete obj;
}

Value VM::pop_stack() {
	Value current_val = stack_top();
	stack.pop_back();
	return current_val;
}

Result VM::interpret(const std::string& input) {
	Parser parser{};
	Compiler compiler(input, parser);

	Function* func = compiler.compile(input);
	if (func == nullptr) return COMPILE_ERROR;

	stack.push_back(make_object(func));
	CallFrame* frame = &frames[frame_count++];
	frame->function = func;
	frame->program_counter = 0;
	frame->slots = &stack[0];
	return run();
}

Value VM::peek(int distance) {
	return stack[stack.size() - 1 - distance];
}

void VM::runtime_error(const std::string& format, ...) {
	//va_list args;
	//va_start(args, format);
	std::cout << format;
	//va_end(args);
	//fputs("\n", stderr);

	CallFrame* frame = &frames[frame_count - 1];

	size_t instruction = &frame->function->chunk.code[program_counter] - &frame->function->chunk.code[0] - 1;
	std::cout << " at line: " << frame->function->chunk.lines[instruction] << std::endl;
	stack = std::vector<Value>();
	frame_count = 0;
}

Value VM::stack_top() {
	return stack[stack.size() - 1];
}

bool VM::values_equal(Value a, Value b) {
	if (a.type != b.type) return false;
	switch (a.type) {
	case VALUE_BOOL:
		return get_bool(a) == get_bool(b);
	case VALUE_VOID:
		return true;
	case VALUE_NUMBER:
		return get_number(a) == get_number(b);
	case VALUE_OBJECT:
	{
		String* a_str = get_string(a);
		String* b_str = get_string(b);
		return a_str->str == b_str->str;
	}
	default:
		return false;
	}
}

void VM::concatenate_string() {
	String* b = get_string(pop_stack());
	String* a = get_string(pop_stack());
	String* new_str = new String(a->str + b->str);
	new_str->next = objects;
	objects = new_str;
	stack.push_back(make_object(new_str));
	delete a, b;
}


