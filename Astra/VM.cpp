#include "Vm.h"

#include <iostream>
#include <cstdarg>
#include <format>
#include <iomanip>

/*template< typename T >
std::string int_to_hex(T i)
{
	std::stringstream stream;
	stream << "0x"
		<< std::setfill('0') << std::setw(sizeof(T) * 2)
		<< std::hex << i;
	return stream.str();
}*/
Value time_native(int arg_count, Value* args) {
	return make_number((double)clock() / CLOCKS_PER_SEC);
}

Value input_native(int arg_count, Value* args) {
	if (arg_count > 0)
		std::cout << get_string(args[1])->str;
	std::string input;
	std::getline(std::cin, input);
	return make_string(new String(input));
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
			push_stack(make_number(a - b));
			break;
		case TOKEN_PLUS:
			push_stack(make_number(a + b));
			break;
		case TOKEN_STAR:
			push_stack(make_number(a * b));
			break;
		case TOKEN_SLASH:
			push_stack(make_number(a / b));
			break;
		case TOKEN_CAP:
			push_stack(make_number(pow(a, b)));
			break;
		case TOKEN_GREATER:
			push_stack(make_bool(a > b));
			break;
		case TOKEN_LESS:
			push_stack(make_bool(a < b));
			break;
		}
		
	} while (false);
	return OK;
}

inline uint8_t read_byte(CallFrame* frame) {
	return frame->closure->function->chunk.code[frame->program_counter++];
}

inline Value read_constant(CallFrame* frame) {
	return frame->closure->function->chunk.constants.values[read_byte(frame)];
}

inline uint16_t read_short(CallFrame* frame) {
	return (uint16_t)(frame->closure->function->chunk.code[frame->program_counter - 2] << 8 | frame->closure->function->chunk.code[frame->program_counter - 1]);
}

Result VM::run() {
	CallFrame* frame = &frames[frames.size() - 1];
	
	while (true) {
		//std::cout << unsigned(frame->function->chunk.code[frame->program_counter]);
		//for (Value& val : frame->slots) {
		//	print_value(val);
		//}
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
		switch (instruction = read_byte(frame)) {
		case OC_RETURN:
		{
			Value res = pop_stack();
			int frame_count = frames.size() - 1;
			
			if (frame_count == 0) {
				pop_stack();
				return OK;
			}

			stack.erase(stack.begin() + frame->stack_start_pos, stack.end());
			push_stack(res);
			frames.pop_back();
			frame = &frames[frames.size() - 1];
			frame->slots = &stack[frame->stack_start_pos];
			break;
		}
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
		case OC_POWER:
			res = binary_operation(VALUE_NUMBER, TOKEN_CAP);
			break;
		case OC_EQUAL:
		{
			Value b = pop_stack();
			Value a = pop_stack();
			push_stack(make_bool(values_equal(a, b)));
			break;
		}
		case OC_GREATER:
			res = binary_operation(VALUE_BOOL, TOKEN_GREATER);
			break;
		case OC_LESS:
			res = binary_operation(VALUE_BOOL, TOKEN_LESS);
			break;
		case OC_NOT:
			push_stack(make_bool(is_false(pop_stack())));
			break;
		case OC_NEGATE:
			if (!is_number(peek(0))) {
				runtime_error("Operand must be a number");
				return RUNTIME_ERROR;
			}
			push_stack(make_number(-get_number(pop_stack())));
			break;
		case OC_CONSTANT:
		{
			Value constant = read_constant(frame);
			push_stack(constant);
			//std::cout << "CONSTANT!";
			//print_value(stack[stack.size() - 1]);
			//std::cout << "WITH";
			//print_value(frame->slots[stack.size() - 1]);
			break;
		}
		case OC_VOID:
			push_stack(make_void());
			break;
		case OC_TRUE:
			push_stack(make_bool(true));
			break;
		case OC_FALSE:
			push_stack(make_bool(false));
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
			String* name = get_string(read_constant(frame));
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
			String* name = get_string(read_constant(frame));
			if (globals.find(name->str) == globals.end()) {
				runtime_error(std::format("Undefined variable '{0}'", name->str));
				return RUNTIME_ERROR;
			}
			push_stack(globals[name->str]);
			break;
		}
		case OC_SET_GLOBAL:
		{
			String* name = get_string(read_constant(frame));
			if (globals.find(name->str) == globals.end()) {
				runtime_error(std::format("Undefined variable '{0}'", name->str));
				return RUNTIME_ERROR;
			}
			globals[name->str] = peek(0);
			break;
		}
		case OC_GET_LOCAL:
		{
			uint8_t slot = read_byte(frame);
			push_stack(frame->slots[slot]);
			break;
		}
		case OC_SET_LOCAL:
		{
			uint8_t slot = read_byte(frame);
			frame->slots[slot] = peek(0);
			break;
		}
		case OC_JMP_IF_FALSE:
		{
			frame->program_counter += 2;
			uint16_t offset = read_short(frame);
			if (is_false(peek(0))) frame->program_counter += offset;
			break;
		}
		case OC_JMP:
		{
			frame->program_counter += 2;
			uint16_t offset = read_short(frame); 
			frame->program_counter += offset;
			break;
		}
		case OC_LOOP:
		{
			frame->program_counter += 2;
			uint16_t offset = read_short(frame); 
			frame->program_counter -= offset;
			break;
		}
		case OC_CALL:
		{
			int arg_count = read_byte(frame);
			if (!call_value(peek(arg_count), arg_count)) {
				return RUNTIME_ERROR;
				break;
			}
			frame = &frames[frames.size() - 1];
			break;
		}
		case OC_CLOSURE:
		{
			Function* func = get_function(read_constant(frame));
			Closure* closure = new Closure(func);
			push_stack(make_object(closure));
			for (int i = 0; i < closure->upvalue_count; i++) {
				uint8_t is_local = read_byte(frame);
				uint8_t index = read_byte(frame);
				if (is_local) {
					
					closure->upvalues.push_back(capture_upvalue(frame->slots + index));
					//print_value(*((Closure*)std::get<Object*>(stack[stack.size() - 1].value))->upvalues[i]->location);
				}
				else
					closure->upvalues.push_back(frame->closure->upvalues[index]);
				CallFrame* frame = &frames[frames.size() - 1];
				frame->slots = &stack[frame->stack_start_pos];
			}
			break;
		}
		case OC_GET_UPVALUE: {
			uint8_t slot = read_byte(frame);
			std::cout << "SLOT" << unsigned(slot);
			print_value(*frame->closure->upvalues[slot]->location);
			push_stack(*frame->closure->upvalues[slot]->location);
			break;
		}
		case OC_SET_UPVALUE: {
			uint8_t slot = read_byte(frame);
			*frame->closure->upvalues[slot]->location = peek(0);
			break;
		}
		}
		
		if (res == RUNTIME_ERROR) {
			return RUNTIME_ERROR;
		}
	}
}

bool VM::call_value(Value callee, int arg_count) {
	if (is_object(callee)) {
		switch (get_object(callee)->type)
		{
		case OBJ_CLOSURE:
			return call_function(get_closure(callee), arg_count);
		case OBJ_NATIVE:
		{
			NativeFn native = get_native(callee)->native_fn;
			Value result = native(arg_count, &stack[stack.size() - 1] - arg_count);
			for (int i = 0; i < arg_count + 1; i++) {
				stack.pop_back();
			}
			push_stack(result);
			return true;
		}
		}
		
	}
	runtime_error("Only functions and classes are callable");
	return false;
}

void VM::define_native(std::string name, NativeFn function) {
	stack.push_back(make_string(new String(name)));
	stack.push_back(make_object(new Native(function)));
	globals[get_string(stack[0])->str] = stack[1];
	stack.pop_back();
	stack.pop_back();
}


bool VM::call_function(Closure* closure, int arg_count) {
	if (arg_count != closure->function->arity) {
		runtime_error(std::format("Expected {0} arguments but got {1}", closure->function->arity, arg_count));
		return false;
	}
	
	/*std::cout << "FRONT: ";

	for (int i = stack.size() - 1 - arg_count; i < stack.size(); i++) {
		std::cout << i;
		print_value(stack[i]);
	}
	std::cout << frames.size() << " " << std::endl;

	std::cout << "BACK: ";

	for (int i = 0; i < stack.size(); i++) {
		std::cout << i;
		print_value(stack[i]);
	}
	std::cout << frames.size() << " " << std::endl;*/

	//for (int i = 0; i < arg_count; i++) {
	//	print_value(*( & (stack[stack.size() - 1]) - arg_count + i));
	//}

	
	
	frames.push_back(CallFrame(closure, 0, &(stack[stack.size() - 1]) - arg_count, stack.size() -1 - arg_count));
	return true;
}

UpvalueObj* VM::capture_upvalue(Value* local) {
	UpvalueObj* created_upvalue = new UpvalueObj(local);
	return created_upvalue;
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
	Layer layer;
	compiler.init_layer(&layer, TYPE_SCRIPT);
	Function* function = compiler.compile();
	if (function == nullptr) return COMPILE_ERROR;
	function->type = OBJ_FUNCTION;

	stack.push_back(make_object(function));
	Closure* closure = new Closure(function);
	pop_stack();
	stack.push_back(make_object(closure));
	call_function(closure, 0);

	return run();
}

Result VM::compile(const std::string& input, std::string& output) {
/*	Parser parser{};
	Compiler compiler(input, parser);
	Chunk new_chunk;
	chunk = &new_chunk;
	if (!compiler.compile(chunk)) {
		chunk->free();
		return COMPILE_ERROR;
	}
	for (uint8_t& byte : chunk->code) {
		//output += int_to_hex(byte) + '\n';
		output += byte + '\n';
	}

	chunk->free();*/
	return OK;
}

Value VM::peek(int distance) {
	return stack[stack.size() - 1 - distance];
}

void VM::runtime_error(std::string format) {
	std::cout << format;

	for (int i = frames.size() - 2; i >= 0; i--) {
		CallFrame* frame = &frames[i];
		Function* func = frame->closure->function;
		size_t instruction = &func->chunk.code[frame->program_counter] - &func->chunk.code[0] - 1;
		std::cout << "in " << (func->name == "" ? "script" : func->name) << " at line: " << func->chunk.lines[instruction] << std::endl;
	}
	stack = std::vector<Value>();
	frames = std::vector<CallFrame>();
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
	push_stack(make_string(new_str));
	delete a, b;
}


