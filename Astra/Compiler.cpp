#include "Compiler.h"
#include "Lexer.h"
#include <iostream>
#include <memory>
#include "Value.h"

Compiler::Compiler(const std::string& src, Parser& _parser) : parser(_parser) {
	lexer.source = src;
	
	rules[TOKEN_L_PAR] = ParseRule{ FN_GROUPING, FN_CALL, PREC_CALL };
	rules[TOKEN_R_PAR] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_L_BRACE] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_R_BRACE] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_COMMA] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_DOT] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_DOUBLE_DOT] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_MINUS] = ParseRule{ FN_UNARY,     FN_BINARY,   PREC_TERM };
	rules[TOKEN_PLUS] = ParseRule{ FN_NONE,     FN_BINARY,   PREC_TERM };
	rules[TOKEN_SEMICOLON] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_SLASH] = ParseRule{ FN_NONE,     FN_BINARY,   PREC_FACTOR };
	rules[TOKEN_STAR] = ParseRule{ FN_NONE,     FN_BINARY,   PREC_FACTOR };
	rules[TOKEN_BANG] = ParseRule{ FN_UNARY,     FN_NONE,   PREC_NONE };
	rules[TOKEN_BANG_EQUAL] = ParseRule{ FN_NONE,     FN_BINARY,   PREC_EQUALITY };
	rules[TOKEN_EQUAL] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_EQUAL_EQUAL] = ParseRule{ FN_NONE,     FN_BINARY,   PREC_EQUALITY };
	rules[TOKEN_GREATER] = ParseRule{ FN_NONE,     FN_BINARY,   PREC_COMPARISON };
	rules[TOKEN_GREATER_EQUAL] = ParseRule{ FN_NONE,     FN_BINARY,   PREC_COMPARISON };
	rules[TOKEN_LESS] = ParseRule{ FN_NONE,     FN_BINARY,   PREC_COMPARISON };
	rules[TOKEN_LESS_EQUAL] = ParseRule{ FN_NONE,     FN_BINARY,   PREC_COMPARISON };
	rules[TOKEN_ID] = ParseRule{ FN_VARIABLE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_STR] = ParseRule{ FN_STRING,     FN_NONE,   PREC_NONE };
	rules[TOKEN_NUM] = ParseRule{ FN_NUMBER,     FN_NONE,   PREC_NONE };
	rules[TOKEN_AND] = ParseRule{ FN_NONE,     FN_AND,   PREC_AND };
	rules[TOKEN_CLASS] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_ELSE] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_FALSE] = ParseRule{ FN_LITERAL,     FN_NONE,   PREC_NONE };
	rules[TOKEN_FOR] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_FUN] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_IF] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_VOID] = ParseRule{ FN_LITERAL,     FN_NONE,   PREC_NONE };
	rules[TOKEN_OR] = ParseRule{ FN_NONE,     FN_OR,   PREC_OR };
	rules[TOKEN_PRINT] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_RETURN] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_SUPER] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_THIS] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_TRUE] = ParseRule{ FN_LITERAL,     FN_NONE,   PREC_NONE };
	rules[TOKEN_VAR] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_WHILE] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_ERROR] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_EOF] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_CAP] = ParseRule{ FN_NONE,     FN_BINARY,   PREC_POWER };
	rules[TOKEN_PLUS_EQUAL] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_MINUS_EQUAL] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_STAR_EQUAL] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_SLASH_EQUAL] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_PLUS_PLUS] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_MINUS_MINUS] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_BREAK] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_CONTINUE] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
}

Function* Compiler::compile() {
	parser.current_token = Token(TOKEN_AND, "", 0);
	/*while (true) {
		Token token = lexer.lex();
		if (token.line != lexer.line) {
			std::cout << "    " << token.line;
			lexer.line = token.line;
		}
		else
			std::cout << "    | ";
		std::cout << token.type << " " << token.value << std::endl;
		if (token.type == TOKEN_EOF) break;
	}
	lexer.pos = -1;
	lexer.line = 0;*/
	current->function = new Function();
	next();
	while (!match(TOKEN_EOF)) {
		declaration();
	}
	Function* function = end();
	return parser.error ? nullptr : function;
	
}

bool Compiler::match(TokenType type) {
	if (parser.current_token.type != type) return false;
	next();
	return true;
}

void Compiler::next() {
	parser.previous_token = parser.current_token;
	
	while (true) {
		
		parser.current_token = lexer.lex();
		
		if (parser.current_token.type != TOKEN_ERROR) break;
		error_at(&parser.current_token, parser.current_token.value);
	}
}

void Compiler::error_at(Token* token, const std::string& message) {
	if (parser.panic) return;
	parser.panic = true;
	std::cout << "Error";

	if (token->type == TOKEN_EOF) {
		std::cout << " at end: ";
	}
	else if (token->type == TOKEN_ERROR) {

	}
	else {
		std::cout << " at line: " << token->line << " ";
	}
	std::cout << message << std::endl;
	parser.error = true;
}

uint8_t Compiler::make_constant(Value value) {
	int constant = current_chunk()->add_constant(value);
	if (constant > UINT8_MAX) {
		error("Too many constants in one chunk");
		return 0;
	}
	return (uint8_t)constant;
}

void Compiler::unary() {
	TokenType operator_type = parser.previous_token.type;

	parse_precedence(PREC_UNARY);
	switch(operator_type) {
	case TOKEN_MINUS:
		emit_byte(OC_NEGATE);
		break;
	case TOKEN_BANG:
		emit_byte(OC_NOT);
		break;
	default:
		return;
	}
}

void Compiler::binary() {
	TokenType operator_type = parser.previous_token.type;
	
	ParseRule rule = get_rule(operator_type);
	parse_precedence((Precedence)(rule.precedence + 1));

	switch (operator_type) {
	case TOKEN_BANG_EQUAL:
		emit_bytes(OC_EQUAL, OC_NOT);
		break;
	case TOKEN_EQUAL_EQUAL:
		emit_byte(OC_EQUAL);
		break;
	case TOKEN_GREATER:
		emit_byte(OC_GREATER);
		break;
	case TOKEN_LESS:
		emit_byte(OC_LESS);
		break;

	case TOKEN_GREATER_EQUAL:
		emit_bytes(OC_LESS, OC_NOT);
		break;
	case TOKEN_LESS_EQUAL:
		emit_bytes(OC_GREATER, OC_NOT);
		break;
	case TOKEN_PLUS:
		emit_byte(OC_ADD);
		break;
	case TOKEN_MINUS:
		emit_byte(OC_SUBTRACT);
		break;
	case TOKEN_STAR:
		emit_byte(OC_MULTIPLY);
		break;
	case TOKEN_SLASH:
		emit_byte(OC_DIVIDE);
		break;
	case TOKEN_CAP:
		emit_byte(OC_POWER);
		break;
	default:
		return;

	}
}

void Compiler::parse_precedence(Precedence precedence) {
	next();
	ParseFn prefix_rule = get_rule(parser.previous_token.type).prefix;
	if (prefix_rule == NULL) {
		error("Expected expression");
		return;
	}
	
	can_assign = precedence <= PREC_ASSIGNMENT;
	call_prec_function(prefix_rule);


	while (precedence <= get_rule(parser.current_token.type).precedence) {
		next();
		ParseFn infix_rule = get_rule(parser.previous_token.type).infix;
		call_prec_function(infix_rule);
	}

	if (can_assign && match(TOKEN_EQUAL)) {
		error("Invalid assignment target");
	}
	else if (can_assign && match(TOKEN_PLUS_EQUAL)) {
		error("Invalid assignment target");
	}
	else if (can_assign && match(TOKEN_MINUS_EQUAL)) {
		error("Invalid assignment target");
	}
	else if (can_assign && match(TOKEN_SLASH_EQUAL)) {
		error("Invalid assignment target");
	}
	else if (can_assign && match(TOKEN_STAR_EQUAL)) {
		error("Invalid assignment target");
	}
}

void Compiler::emit_bytes(uint8_t b1, uint8_t b2) {
	emit_byte(b1);
	emit_byte(b2);
}

Function* Compiler::end() {
	emit_return();
	Function* function = current->function;
	//#ifdef DEBUG_PRINT_CODE
	if (!parser.error) {
		current_chunk()->disassemble(function->name != "" ? function->name : "<script>");
	}
	//#endif
	current = current->enclosing;
	return function;
}

void Compiler::call_prec_function(ParseFn func) {
	switch (func)
	{
	case FN_NONE:
		return;
	case FN_GROUPING:
		return grouping();
	case FN_UNARY:
		return unary();
	case FN_BINARY:
		return binary();
	case FN_NUMBER:
		return number();
	case FN_LITERAL:
		return literal();
	case FN_STRING:
		return string();
	case FN_VARIABLE:
		return variable();
	case FN_AND:
		return and_operation();
	case FN_OR:
		return or_operation();
	case FN_CALL:
		return function_call();
	default:
		break;
	}
}

void Compiler::function_call() {
	uint8_t arg_count = argument_list();
	emit_bytes(OC_CALL, arg_count);
}

uint8_t Compiler::argument_list() {
	uint8_t arg_count = 0;
	if (parser.current_token.type != TOKEN_R_PAR) {
		do {
			expression();
			if (arg_count == 255) {
				error("Can't pass more than 255 arguments.");
			}
			arg_count++;
		} while (match(TOKEN_COMMA));
	}
	consume(TOKEN_R_PAR, "Expect ')' after arguments.");
	return arg_count;
}

void Compiler::grouping() {
	expression();
	consume(TOKEN_R_PAR, "Expected ')'");
}

void Compiler::number() {
	double value = stod(parser.previous_token.value);
	emit_constant(make_number(value));
}

void Compiler::expression() {
	parse_precedence(PREC_ASSIGNMENT);
}

void Compiler::consume(TokenType type, const std::string& message) {
	if (parser.current_token.type == type) {
		next();
		return;
	}
	error_at(&parser.current_token, message);
}

void Compiler::literal() {
	switch (parser.previous_token.type) {
	case TOKEN_FALSE:
		emit_byte(OC_FALSE);
		break;
	case TOKEN_VOID:
		emit_byte(OC_VOID);
		break;
	case TOKEN_TRUE:
		emit_byte(OC_TRUE);
		break;
	default:
		return;
	}
}

void Compiler::string() {
	emit_constant(make_string(new String(parser.previous_token.value)));
}

void Compiler::declaration() {
	if (match(TOKEN_VAR)) {
		variable_declaration();
	}
	else if (match(TOKEN_FUN)) {
		function_declaration();
	}
	else statement();
	
	if (parser.panic) synchronize();
}

void Compiler::function_declaration() {
	uint8_t global = parse_variable("Expected function name");
	make_initialized();
	make_function(TYPE_FUNCTION);
	define_variable(global);
}

void Compiler::make_function(FunctionType type) {
	Layer layer;
	init_layer(&layer, type);
	new_scope();
	consume(TOKEN_L_PAR, "Expected '('");
	if (parser.current_token.type != TOKEN_R_PAR) {
		do {
			current->function->arity++;
			if (current->function->arity > 255) {
				error("Can't have more than 255 parameters");
			}
			uint8_t constant = parse_variable("Expected parameter name");
			define_variable(constant);
		} while (match(TOKEN_COMMA));
	}
	consume(TOKEN_R_PAR, "Expected ')'");
	consume(TOKEN_L_BRACE, "Expected '{'");
	compount_statement();

	Function* function = end();
	function->type = OBJ_FUNCTION;
	emit_bytes(OC_CLOSURE, make_constant(make_object(function)));

	for (int i = 0; i < function->upvalue_count; i++) {
		emit_byte(layer.upvalues[i].is_local ? 1 : 0);
		emit_byte(layer.upvalues[i].index);
	}
}

void Compiler::statement() {
	if (match(TOKEN_PRINT)) {
		print_statement();
	}
	else if (match(TOKEN_RETURN)) {
		return_statement();
	}
	else if (match(TOKEN_BREAK)) {
		break_statement();
	}
	else if (match(TOKEN_CONTINUE)) {
		continue_statement();
	}
	else if (match(TOKEN_IF)) {
		if_statement();
	}
	else if (match(TOKEN_WHILE)) {
		while_statement();
	}
	else if (match(TOKEN_FOR)) {
		for_statement();
	}
	else if (match(TOKEN_L_BRACE)) {
		new_scope();
		compount_statement();
		end_scope();
	}
	else {
		expression_statement();
	}
}

void Compiler::return_statement() {
	if (current->function_type == TYPE_SCRIPT) {
		error("Can't return from script");
	}
	if (match(TOKEN_SEMICOLON)) {
		emit_return();
		return;
	}
	expression();
	consume(TOKEN_SEMICOLON, "Expected ';'");
	emit_byte(OC_RETURN);
}

void Compiler::expression_statement() {
	expression();
	consume(TOKEN_SEMICOLON, "Expected ';' after expression");
	emit_byte(OC_POP);
}

void Compiler::print_statement() {
	expression();
	consume(TOKEN_SEMICOLON, "Expected ';' after expression");
	emit_byte(OC_PRINT);
}

void Compiler::synchronize() {
	parser.panic = false;

	while (parser.current_token.type != TOKEN_EOF) {
		if (parser.previous_token.type == TOKEN_SEMICOLON) return;
		switch (parser.current_token.type) {
		case TOKEN_CLASS:
		case TOKEN_FUN:
		case TOKEN_VAR:
		case TOKEN_FOR:
		case TOKEN_IF:
		case TOKEN_WHILE:
		case TOKEN_PRINT:
		case TOKEN_RETURN:
			return;
		default:
			;
		}

		next();
	}
}

void Compiler::variable_declaration() {
	uint8_t global = parse_variable("Expected variable name");

	if(match(TOKEN_EQUAL)) {
		expression();
	}
	else {
		emit_byte(OC_VOID);
	}

	consume(TOKEN_SEMICOLON, "Expected ';' after variable declaration");

	define_variable(global);
}

uint8_t Compiler::parse_variable(const std::string& error) {
	consume(TOKEN_ID, error);

	declare_variable();
	if (current->scope_depth > 0) return 0;

	return make_constant(make_string(new String(parser.previous_token.value)));
}

void Compiler::define_variable(uint8_t global) {
	if (current->scope_depth > 0) {
		make_initialized();
		return;
	}
	emit_bytes(OC_DEFINE_GLOBAL, global);
}

void Compiler::variable() {
	named_variable(parser.previous_token);
}

void Compiler::named_variable(Token name) {
	
	uint8_t get_op, set_op;
	int arg = resolve_local(current, name);
	if (arg != -1) {
		get_op = OC_GET_LOCAL;
		set_op = OC_SET_LOCAL;
	}
	else if ((arg = resolve_upvalue(current, name)) != -1) {
		get_op = OC_GET_UPVALUE;
		set_op = OC_SET_UPVALUE;
	}
	else {
		arg = identifier_constant(&name);
		get_op = OC_GET_GLOBAL;
		set_op = OC_SET_GLOBAL;
	}
	

	if (can_assign && match(TOKEN_EQUAL)) {
		expression();
		emit_bytes(set_op, (uint8_t)arg);
	}
	else if (can_assign && match(TOKEN_PLUS_EQUAL)) {
		emit_bytes(get_op, (uint8_t)arg);
		expression();
		emit_byte(OC_ADD);
		emit_bytes(set_op, (uint8_t)arg);
	}
	else if (can_assign && match(TOKEN_MINUS_EQUAL)) {
		emit_bytes(get_op, (uint8_t)arg);
		expression();
		emit_byte(OC_SUBTRACT);
		emit_bytes(set_op, (uint8_t)arg);
	}
	else if (can_assign && match(TOKEN_STAR_EQUAL)) {
		emit_bytes(get_op, (uint8_t)arg);
		expression();
		emit_byte(OC_MULTIPLY);
		emit_bytes(set_op, (uint8_t)arg);
	}
	else if (can_assign && match(TOKEN_SLASH_EQUAL)) {
		emit_bytes(get_op, (uint8_t)arg);
		expression();
		emit_byte(OC_DIVIDE);
		emit_bytes(set_op, (uint8_t)arg);
	}
	else {
		emit_bytes(get_op, (uint8_t)arg);
	}
}

int Compiler::resolve_upvalue(Layer* layer, Token& name) {
	if (layer->enclosing == nullptr) return -1;

	int local = resolve_local(layer->enclosing, name);
	if (local != -1) {
		layer->enclosing->locals[local].is_captured = true;
		return add_upvalue(layer, (uint8_t)local, true);
	}

	int upvalue = resolve_upvalue(layer->enclosing, name);
	if (upvalue != -1) {
		return add_upvalue(layer, (uint8_t)upvalue, false);
	}
	return -1;
}

int Compiler::add_upvalue(Layer* layer, uint8_t index, bool is_local) {
	int upvalue_count = layer->function->upvalue_count;
	for (int i = 0; i < upvalue_count; i++) {
		Upvalue* upvalue = &layer->upvalues[i];
		if (upvalue->index == index && upvalue->is_local == is_local)
			return i;
			
	}
	
	layer->upvalues.push_back(Upvalue(index, is_local));
	return layer->function->upvalue_count++;
}

uint8_t Compiler::identifier_constant(Token* name) {
	return make_constant(make_string(new String(name->value)));
}

void Compiler::new_scope() {
	current->scope_depth++;
}
void Compiler::compount_statement() {
	while (parser.current_token.type != TOKEN_R_BRACE && parser.current_token.type != TOKEN_EOF) {
		declaration();
	}

	consume(TOKEN_R_BRACE, "Expected '}'");
}
void Compiler::end_scope() {
	current->scope_depth--;

	while (current->local_count > 0 && current->locals[current->local_count - 1].depth > current->scope_depth) {
		if (current->locals[current->local_count - 1].is_captured) {
			emit_byte(OC_CLOSE_UPVALUE);
		}
		else
			emit_byte(OC_POP);
		current->local_count--;
	}
}

void Compiler::declare_variable() {
	
	if (current->scope_depth == 0) return;
	Token name = parser.previous_token;
	for (int i = current->local_count - 1; i >= 0; i--) {
		Local* local = &current->locals[i];
		if (local->depth != -1 && local->depth < current->scope_depth) break;

		if (equal_identifiers(name, local->name)) error("Variable already exists in scope");
	}
	
	add_local(current, name);
}

void Compiler::add_local(Layer* layer, Token& name) {
	if (layer->local_count == UINT8_MAX + 1) {
		error("Too many local variables in scope");
		return;
	}

	Local* local = &layer->locals[layer->local_count++];
	local->name = name;
	local->depth = -1;
	local->depth = layer->scope_depth;
}

bool Compiler::equal_identifiers(Token& a, Token& b) {
	if (a.value == b.value) return true;
	return false;
}

int Compiler::resolve_local(Layer* layer, Token& name) {
	for (int i = layer->local_count - 1; i >= 0; i--) {
		Local* local = &layer->locals[i];
		
		if (equal_identifiers(name, local->name)) {
			if (local->depth == -1) error("Tried accessing variable in it's initialization");
			
			return i;
			
		}
	}

	return -1;
}

void Compiler::make_initialized() {
	if (current->scope_depth == 0) return;
	current->locals[current->local_count - 1].depth = current->scope_depth;
}

void Compiler::if_statement() {
	expression();
	
	int then_jmp = emit_jump(OC_JMP_IF_FALSE);
	emit_byte(OC_POP);
	
	statement();

	int else_jmp = emit_jump(OC_JMP);

	patch_jump(then_jmp);
	emit_byte(OC_POP);
	if (match(TOKEN_ELSE)) statement();
	patch_jump(else_jmp);
}

int Compiler::emit_jump(uint8_t instruction, int pos) {
	emit_byte(instruction);
	emit_byte(0xff);
	emit_byte(0xff);
	if(pos==-1)
		return current_chunk()->code.size() - 2;
	return pos - 2;
}

void Compiler::patch_jump(int offset) {
	int jump = current_chunk()->code.size() - offset - 2;

	if (jump > UINT16_MAX) {
		error("Can't jump this big");
	}

	current_chunk()->code[offset] = (jump >> 8) & 0xff;
	current_chunk()->code[offset + 1] = jump & 0xff;
}

void Compiler::and_operation() {
	int end_jump = emit_jump(OC_JMP_IF_FALSE);

	emit_byte(OC_POP);
	parse_precedence(PREC_AND);

	patch_jump(end_jump);
}

void Compiler::or_operation() {
	int else_jump = emit_jump(OC_JMP_IF_FALSE);
	int end_jump = emit_jump(OC_JMP);

	patch_jump(else_jump);
	emit_byte(OC_POP);


	parse_precedence(PREC_AND);
	patch_jump(end_jump);
}

void Compiler::while_statement() {
	int surrounding_loop_start = current_loop_start;
	int surrounding_scope_depth = current_loop_scope_depth;
	current_loop_start = current_chunk()->code.size();
	current_loop_scope_depth = current->scope_depth;
	expression();

	int exit_jump = emit_jump(OC_JMP_IF_FALSE);
	current_exit_jump = exit_jump;
	emit_byte(OC_POP);
	statement();
	emit_loop(current_loop_start);
	
	patch_jump(exit_jump);
	
	emit_byte(OC_POP);
	if (break_pos != -1) {
		patch_jump(break_pos);
		break_pos = -1;
	}
	current_loop_start = surrounding_loop_start;
	current_loop_scope_depth = surrounding_scope_depth;
}

void Compiler::break_statement() {
	if (current_loop_start == -1) {
		error("Break statement outside of loop");
		return;
	}
	consume(TOKEN_SEMICOLON, "Expected ';' after expression");
	for (int i = current->local_count - 1; i >= 0 && current->locals[i].depth > current_loop_scope_depth; i--)
		emit_byte(OC_POP);
	break_pos = emit_jump(OC_JMP);
	emit_byte(OC_POP);
}

void Compiler::continue_statement() {
	if (current_loop_start == -1) {
		error("Continue statement outside of loop");
		return;
	}
	consume(TOKEN_SEMICOLON, "Expected ';' after expression");
	for (int i = current->local_count - 1; i >= 0 && current->locals[i].depth > current_loop_scope_depth; i--)
		emit_byte(OC_POP);
	
	emit_loop(current_loop_start);
}

void Compiler::emit_loop(int loop_start) {
	emit_byte(OC_LOOP);

	int offset = current_chunk()->code.size() - loop_start + 2;
	if (offset > UINT16_MAX) error("Loop body is too large");

	emit_byte((offset >> 8) & 0xff);
	emit_byte(offset & 0xff);
}

void Compiler::for_statement() {
	new_scope();

	if (match(TOKEN_SEMICOLON));
	else if (match(TOKEN_VAR)) {
		variable_declaration();
	}
	else {
		expression_statement();
	}
	int surrounding_loop_start = current_loop_start;
	int surrounding_scope_depth = current_loop_scope_depth;
	current_loop_start = current_chunk()->code.size();
	current_loop_scope_depth = current->scope_depth;
	int exit_jump = -1;
	if (!match(TOKEN_SEMICOLON)) {
		expression();
		consume(TOKEN_SEMICOLON, "Expected ';' after condition");

		exit_jump = emit_jump(OC_JMP_IF_FALSE);
		current_exit_jump = exit_jump;
		emit_byte(OC_POP);
	}
	if (!match(TOKEN_DOUBLE_DOT)) {
		int body_jump = emit_jump(OC_JMP);
		int increment_start = current_chunk()->code.size();
		expression();
		consume(TOKEN_DOUBLE_DOT, "Expected ':'");
		emit_byte(OC_POP);

		emit_loop(current_loop_start);
		current_loop_start = increment_start;
		patch_jump(body_jump);
	}
	statement();
	emit_loop(current_loop_start);
	if (exit_jump != -1) {
		patch_jump(exit_jump);
		emit_byte(OC_POP);
	}
	if (break_pos != -1) {
		patch_jump(break_pos);
		break_pos = -1;
	}

	current_loop_start = surrounding_loop_start;
	current_loop_scope_depth = surrounding_scope_depth;

	end_scope();
}