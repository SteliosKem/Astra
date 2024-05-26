#include "Vm.h"
#include "Memory.h"
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

Value list_native(int arg_count, Value* args) {
	List* list = new List();
	for (int i = 0; i < arg_count; i++) {
		list->values.push_back(args[i]);
	}
	return make_object(list);
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
	//return frame->closure->function->chunk.code[frame->program_counter++];
	return *frame->program_counter++;
}

inline Value read_constant(CallFrame* frame) {
	return frame->closure->function->chunk.constants.values[read_byte(frame)];
}

inline uint16_t read_short(CallFrame* frame) {
	//return (uint16_t)(frame->closure->function->chunk.code[frame->program_counter - 2] << 8 | frame->closure->function->chunk.code[frame->program_counter - 1]);
	return (uint16_t)((frame->program_counter[-2] << 8) | frame->program_counter[-1]);
}

Result VM::run() {
	//CallFrame* frame = &frames[frames.size() - 1];
	CallFrame* frame = &frames[frame_count - 1];
	while (true) {
		//if (frame->closure->upvalues.size() > 0)
		//	print_value(*frame->closure->upvalues[0]->location);
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
			close_upvalues(frame->slots);
			//int frame_count = frames.size() - 1;
			frame_count--;
			
			if (frame_count == 0) {
				pop_stack();
				return OK;
			}

			//stack.erase(stack.begin() + frame->stack_start_pos, stack.end());
			//push_stack(res);
			//frames.pop_back();
			//frame = &frames[frames.size() - 1];
			//frame->slots = &stack[frame->stack_start_pos];
			stack_top = frame->slots;
			push_stack(res);
			frame = &frames[frame_count - 1];
			break;
		}
		case OC_ADD:
		{
			if (is_string(peek(0)) || is_string(peek(1))) concatenate_string();
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
				runtime_error(std::format("Cannot set undefined variable '{0}'", name->str));
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
			frame = &frames[frame_count - 1];
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
				}
				else
					closure->upvalues.push_back(frame->closure->upvalues[index]);
			}
			break;
		}
		case OC_GET_UPVALUE: {
			uint8_t slot = read_byte(frame);
			push_stack(*frame->closure->upvalues[slot]->location);
			break;
		}
		case OC_SET_UPVALUE: {
			uint8_t slot = read_byte(frame);
			*frame->closure->upvalues[slot]->location = peek(0);
			break;
		}
		case OC_CLOSE_UPVALUE: {
			close_upvalues(stack_top - 1);
			pop_stack();
			break;
		}
		case OC_CLASS:
			push_stack(make_object(new ClassObj(get_string(read_constant(frame))->str)));
			break;
		case OC_ENUM:
			push_stack(make_object(new Enumeration(get_string(read_constant(frame))->str)));
			break;
		case OC_GET_MEMBER: {
			if (is_enum(peek(0))) {
				Enumeration* _enum = get_enum(peek(0));
				std::string name = get_string(read_constant(frame))->str;
				Value value;

				if (std::count(_enum->values.begin(), _enum->values.end(), name) > 0) {
					value = make_object(new EnumValue(name, _enum));
					pop_stack();
					push_stack(value);
					break;
				}
				else {
					runtime_error("Enum '" + _enum->name + "' does not contain value '" + name + "'");
					return RUNTIME_ERROR;
				}
				break;
			}
			if (!is_instance(peek(0))) {
				runtime_error("Non-instances do not have fields");
				return RUNTIME_ERROR;
			}

			Instance* instance = get_instance(peek(0));
			std::string& name = get_string(read_constant(frame))->str;
			Value value;

			if (instance->fields.find(name) != instance->fields.end()) {
				value = instance->fields[name];
				pop_stack();
				push_stack(value);
				break;
			}
			
			if (!bind_method(instance->class_target, name))
				return RUNTIME_ERROR;
			break;
		}
		case OC_SET_MEMBER: {
			if (!is_instance(peek(1))) {
				runtime_error("Can't set field of non-instance");
				return RUNTIME_ERROR;
			}
			Instance* instance = get_instance(peek(1));
			instance->fields[get_string(read_constant(frame))->str] = peek(0);
			Value value = pop_stack();
			pop_stack();
			push_stack(value);
			break;
		}
		case OC_GET_MEMBER_COMPOUND: {
			Instance* instance = get_instance(peek(0));
			std::string& name = get_string(read_constant(frame))->str;
			push_stack(instance->fields[name]);
			break;
		}
		case OC_METHOD:
			define_method(get_string(read_constant(frame))->str);
			break;
		case OC_ENUM_VALUE:
			define_enum_value(get_string(read_constant(frame))->str);
			break;
		case OC_INVOKE: {
			std::string method = get_string(read_constant(frame))->str;
			int arg_count = read_byte(frame);
			if (!invoke(method, arg_count)) {
				return RUNTIME_ERROR;
			}
			frame = &frames[frame_count - 1];
			break;
		}
		case OC_INHERIT: {
			Value superclass = peek(1);
			if (!is_class(superclass)) {
				runtime_error("Superclass must be a class");
				return RUNTIME_ERROR;
			}
			ClassObj* subclass = get_class(peek(0));
			for (auto& [key, value] : get_class(superclass)->methods) {
				subclass->methods[key] = value;
			}
			pop_stack();
			break;
		}
		case OC_GET_SUPER: {
			std::string name = get_string(read_constant(frame))->str;
			ClassObj* superclass = get_class(pop_stack());

			if (!bind_method(superclass, name)) {
				return RUNTIME_ERROR;
			}
			break;
		}
		case OC_SUPER_INVOKE: {
			std::string method = get_string(read_constant(frame))->str;
			int arg_count = read_byte(frame);
			ClassObj* superclass = get_class(pop_stack());
			if (!invoke_from_class(superclass, method, arg_count)) {
				return RUNTIME_ERROR;
			}
			frame = &frames[frame_count - 1];
			break;
		}
		case OC_GET_INDEX: {
			if (!is_list(peek(1))) {
				runtime_error("Cannot access element of non-list");
				return RUNTIME_ERROR;
			}

			List* list = get_list(peek(1));
			Value value;

			double idx = get_number(pop_stack());
			if (idx >= list->values.size()) {
				runtime_error("Index out of bounds");
				return RUNTIME_ERROR;
			}
			pop_stack();
			push_stack(list->values[(int)idx]);
			break;
		}
		case OC_GET_INDEX_COMPOUND: {
			if (!is_list(peek(1))) {
				runtime_error("Cannot access element of non-list");
				return RUNTIME_ERROR;
			}

			List* list = get_list(peek(1));
			Value value;

			double idx = get_number(peek(0));
			if (idx >= list->values.size()) {
				runtime_error("Index out of bounds");
				return RUNTIME_ERROR;
			}
			push_stack(list->values[(int)idx]);
			break;
		}
		case OC_SET_INDEX: {
			if (!is_list(peek(2))) {
				runtime_error("Cannot set element of non-list");
				return RUNTIME_ERROR;
			}

			List* list = get_list(peek(2));
			Value value;

			Value to_set = pop_stack();
			double idx = get_number(pop_stack());
			if (idx >= list->values.size()) {
				runtime_error("Index out of bounds");
				return RUNTIME_ERROR;
			}
			list->values[idx] = to_set;
			pop_stack();
			break;
		}
		}
		
		
		if (res == RUNTIME_ERROR) {
			return RUNTIME_ERROR;
		}
		
	}

}

bool VM::invoke(std::string name, int arg_count) {
	Value receiver = peek(arg_count);

	if (is_list(receiver)) {
		if (name == "add") {
			List* list = get_list(receiver);
			for (int i = 0; i < arg_count; i++) {
				list->values.push_back(peek(arg_count - i - 1));
			}
			for (int i = 0; i < arg_count; i++) {
				pop_stack();
			}
			
			//pop_stack();
		}
		else if (name == "size") {
			List* list = get_list(receiver);
			if (arg_count > 0) {
				runtime_error("size() does not take arguments");
				return false;
			}
			
			pop_stack();
			push_stack(make_number(list->values.size()));
		}
		else if (name == "pop") {
			List* list = get_list(receiver);
			if (arg_count > 0) {
				runtime_error("pop() does not take arguments");
				return false;
			}

			list->values.pop_back();
		}
		else if (name == "clear") {
			List* list = get_list(receiver);
			if (arg_count > 0) {
				runtime_error("clear() does not take arguments");
				return false;
			}

			list->values.clear();
		}
		else {
			runtime_error("Unknown method " + name);
			return false;
		}
		return true;
	}

	if (!is_instance(receiver)) {
		runtime_error("Only instances have methods");
		return false;
	}

	Instance* instance = get_instance(receiver);
	if (name == "construct") {
		runtime_error("Can't call the constructor");
		return false;
	}
	if (instance->fields.find(name) != instance->fields.end()) {
		Value value = instance->fields[name];
		stack_top[-arg_count - 1] = value;
		return call_value(value, arg_count);
	}
	return invoke_from_class(instance->class_target, name, arg_count);
}

bool VM::invoke_from_class(ClassObj* _class, std::string name, int arg_count) {
	Value method;
	if (_class->methods.find(name) == _class->methods.end()) {
		runtime_error("Undefined method '" + name + "'");
		return false;
	}
	method = _class->methods[name];
	return call_function(get_closure(method), arg_count);
}

bool VM::bind_method(ClassObj* _class, std::string name) {
	Value method;
	if (_class->methods.find(name) == _class->methods.end()) {
		runtime_error("Undefined method '" + name + "'");
		return false;
	}
	method = _class->methods[name];
	BoundMethod* bound = new BoundMethod(peek(0), get_closure(method));
	pop_stack();
	push_stack(make_object(bound));
	return true;
}

void VM::define_method(std::string name) {
	Value method = peek(0);
	ClassObj* _class = get_class(peek(1));
	_class->methods[name] = method;
	pop_stack();
}

void VM::define_enum_value(std::string name) {
	Enumeration* enumeration = get_enum(peek(0));
	enumeration->values.push_back(name);
	//pop_stack();
}

bool VM::call_value(Value callee, int arg_count) {
	if (is_object(callee)) {
		switch (get_object(callee)->type)
		{
		case OBJ_CLOSURE:
			return call_function(get_closure(callee), arg_count);
		case OBJ_CLASS: {
			ClassObj* _class = get_class(callee);
			stack_top[-arg_count - 1] = make_object(new Instance(_class));
			Value initializer;
			if (_class->methods.find("construct") != _class->methods.end()) {
				return call_function(get_closure(_class->methods["construct"]), arg_count);
			}
			else if (arg_count != 0) {
				runtime_error("Expected 0 arguments but got " + std::to_string(arg_count));
				return false;
			}
			return true;
		}
		case OBJ_BOUND_METHOD: {
			BoundMethod* bound = get_bound_method(callee);
			stack_top[-arg_count - 1] = bound->reciever;
			return call_function(bound->method, arg_count);
		}
		case OBJ_NATIVE:
		{
			NativeFn native = get_native(callee)->native_fn;
			Value result = native(arg_count, stack_top - arg_count);
			for (int i = 0; i < arg_count + 1; i++) {
				pop_stack();
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
	push_stack(make_string(new String(name)));
	push_stack(make_object(new Native(function)));
	globals[get_string(stack[0])->str] = stack[1];
	pop_stack();
	pop_stack();
}


bool VM::call_function(Closure* closure, int arg_count) {
	if (arg_count != closure->function->arity) {
		runtime_error(std::format("Expected {0} arguments but got {1}", closure->function->arity, arg_count));
		return false;
	}
	
	CallFrame* frame = &frames[frame_count++];
	frame->closure = closure;
	frame->program_counter = &closure->function->chunk.code[0];
	frame->slots = stack_top - arg_count -1;
	return true;
}

UpvalueObj* VM::capture_upvalue(Value* local) {
	UpvalueObj* previous = nullptr;
	UpvalueObj* upvalue = open_upvalues;
	while (upvalue != nullptr && upvalue->location > local) {
		previous = upvalue;
		upvalue = upvalue->next;
	}

	if (upvalue != nullptr && upvalue->location == local)
		return upvalue;

	UpvalueObj* created_upvalue = new UpvalueObj(local);
	created_upvalue->next = upvalue;

	if (previous == nullptr)
		open_upvalues = created_upvalue;
	else
		previous->next = created_upvalue;
	return created_upvalue;
}

void VM::close_upvalues(Value* last) {
	while (open_upvalues != nullptr && open_upvalues->location >= last) {
		UpvalueObj* upvalue = open_upvalues;
		upvalue->closed = *upvalue->location;
		upvalue->location = &upvalue->closed;
		open_upvalues = upvalue->next;
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

Result VM::interpret(const std::string& input) {
	Parser parser{};
	Compiler comp(input, parser);
	compiler = &comp;
	Layer layer;
	compiler->init_layer(&layer, TYPE_SCRIPT);
	Function* function = compiler->compile();
	if (function == nullptr) return COMPILE_ERROR;
	function->type = OBJ_FUNCTION;

	push_stack(make_object(function));
	Closure* closure = new Closure(function);
	pop_stack();
	push_stack(make_object(closure));
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
	return stack_top[-1 - distance];
}

void VM::runtime_error(std::string format) {
	std::cout << '\n' << "Runtime Error: " << format << '\n';

	CallFrame* frame = &frames[frame_count - 1];
	Function* func = frame->closure->function;
	size_t instruction = frame->program_counter - &func->chunk.code[0] - 1;
	Position pos = func->chunk.lines[instruction];

	std::string& src = compiler->lexer.source;
	const int max_length_til_newline = 40;
	const int max_error_length = 20;
	const int error_size_before_after = 10;
	const int max_length_after = 40;
	std::string before = src.substr(0, pos.start_pos);
	auto last_new_line = before.rfind('\n');
	int last_newline_pos = std::string::npos != last_new_line ? last_new_line : 0;

	int before_error_size = 0;
	int error_size = 0;

	std::string line_str = "Line " + std::to_string(pos.line) + ": ";
	std::cout << "Line " << pos.line << ": ";
	int starting = line_str.size();

	if (pos.start_pos - last_newline_pos > max_length_til_newline) {
		std::string new_str = "..." + src.substr(pos.start_pos - max_length_til_newline, max_length_til_newline);
		std::cout << new_str;
		before_error_size = new_str.size();
	}
	else {
		std::string new_str = src.substr(last_newline_pos, pos.start_pos);
		std::cout << new_str;
		before_error_size = new_str.size();
	}
	if (pos.end_pos - pos.start_pos > max_error_length) {
		std::string new_str = src.substr(pos.start_pos, max_error_length / 4) + "..." + src.substr(pos.end_pos - max_error_length / 4, max_error_length / 4 + 1);
		std::cout << new_str;
		error_size = new_str.size();
	}
	else {
		std::string new_str = src.substr(pos.start_pos, pos.end_pos - pos.start_pos + 1);
		std::cout << new_str;
		error_size = new_str.size();
	}
	auto final_new_line = before.find('\n', pos.end_pos);
	std::string after = "";
	after = final_new_line != std::string::npos ? src.substr(pos.end_pos + 1, final_new_line) : src.substr(pos.end_pos + 1);

	if (after.size() > max_length_after) {
		std::cout << after.substr(0, max_length_after) << "...";
	}
	else
		std::cout << after;
	std::cout << std::endl;
	for (int i = 0; i < starting + before_error_size; i++) {
		std::cout << ' ';
	}
	for (int i = 0; i < error_size; i++) {
		std::cout << '^';
	}
	if (error_size == 0)
		std::cout << '^';

	std::cout << " <- Here" << std::endl;

	for (int i = frame_count - 1; i >= 0; i--) {
		CallFrame* frame = &frames[i];
		Function* func = frame->closure->function;
		size_t instruction = frame->program_counter - &func->chunk.code[0] - 1;
		std::cout << "In " << (func->name == "" ? "script" : func->name) << " at line: " << func->chunk.lines[instruction].line << std::endl;
	}
	stack_top = stack;
	frame_count = 0;
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
		Object* a_obj = get_object(a);
		Object* b_obj = get_object(b);
		if (a_obj->type != b_obj->type)
			return false;
		switch (a_obj->type) {
		case OBJ_STRING: {
			String* a_str = get_string(a);
			String* b_str = get_string(b);
			return a_str->str == b_str->str;
		}
		case OBJ_CLASS: {								// TO DO LATER
			break;
		}
		case OBJ_ENUM_VAL: {
			EnumValue* a_val = get_enum_val(a);
			EnumValue* b_val = get_enum_val(b);
			if (a_val->enumeration != b_val->enumeration)
				return false;
			if (a_val->value == b_val->value)
				return true;
			return false;
		}
		}
		String* a_str = get_string(a);
		String* b_str = get_string(b);
		return a_str->str == b_str->str;
	}
	default:
		return false;
	}
}

void VM::concatenate_string() {
	if (is_string(peek(0)) && is_string(peek(1))) {
		String* b = get_string(pop_stack());
		String* a = get_string(pop_stack());
		String* new_str = new String(a->str + b->str);
		new_str->next = objects;
		objects = new_str;
		push_stack(make_string(new_str));
		//delete a, b;
	}
	else {
		if (is_string(peek(0))) {
			String* b = get_string(pop_stack());
			Value a = pop_stack();
			std::string to_concat = "";
			switch (a.type) {
			case VALUE_VOID:
				to_concat = "void";
				break;
			case VALUE_NUMBER:
				to_concat = std::to_string(get_number(a));
				break;
			case VALUE_BOOL:
				if(get_bool(a))
					to_concat = "true";
				else
					to_concat = "false";
				break;
			default:
				if (!is_instance(a)) {					// To support class to_str built-in
					runtime_error("Cannot perform operation on these values");
					return;
				}
			}
			String* new_str = new String(to_concat + b->str);
			new_str->next = objects;
			objects = new_str;
			push_stack(make_string(new_str));
			//delete b;
		}
		else {
			Value b = pop_stack();
			String* a = get_string(pop_stack());
			std::string to_concat = "";
			switch (b.type) {
			case VALUE_VOID:
				to_concat = "void";
				break;
			case VALUE_NUMBER:
				to_concat = std::to_string(get_number(b));
				break;
			case VALUE_BOOL:
				if (get_bool(b))
					to_concat = "true";
				else
					to_concat = "false";
				break;
			default:
				if (!is_instance(b)) {					// To support class to_str built-in
					runtime_error("Cannot perform operation on these values");
					return;
				}
			}
			String* new_str = new String(a->str + to_concat);
			new_str->next = objects;
			objects = new_str;
			push_stack(make_string(new_str));
		}
	}
}


void VM::collect_garbage() {
	std::cout << "Start Garbage Collector";
	mark_roots();
	std::cout << "End Garbage Collector";
}

void VM::mark_roots() {
	for (Value* slot = stack; slot < stack_top; slot++) {
		Memory::mark_value(*slot);
	}

	for (int i = 0; i < frame_count; i++) {
		Memory::mark_object((Object*)frames[i].closure);
	}
	for (UpvalueObj* upvalue = open_upvalues; upvalue != nullptr; upvalue = upvalue->next) {
		Memory::mark_object((Object*)upvalue);
	}

	Memory::mark_table(globals);
	compiler->mark_compiler_roots();
}

void Memory::mark_value(Value value) {
	if (is_object(value)) mark_object(get_object(value));
}

void Memory::mark_object(Object* object) {
	if (object == nullptr) return;

	std::cout << "mark " << (void*)object;
	print_value(make_object(object));
	std::cout << std::endl;

	object->is_marked = true;

	//gray_stack.push_back(object);
}

void Memory::mark_table(std::unordered_map<std::string, Value>& map) {
	for (auto& [key, value] : map) { 
		mark_value(value);
	}
}