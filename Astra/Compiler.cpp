#include "Compiler.h"
#include "Lexer.h"
#include <iostream>
#include <memory>
#include "Value.h"

Compiler::Compiler(const std::string& src, Parser& _parser) : parser(_parser) {
	lexer.source = src;
	function = new Function();
	Local* local = &locals[local_count++];
	local->depth = 0;
	local->name.value = "";
	rules[TOKEN_L_PAR] = ParseRule{ FN_GROUPING, FN_NONE, PREC_NONE };
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
}

Function* get_function(Value val) {
	return (Function*)get_object(val);
}

Function* Compiler::compile(std::string src) {
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
	next();
	while (!match(TOKEN_EOF)) {
		declaration();
	}
	Function* func = end();
	return parser.error ? nullptr : func;
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
}

void Compiler::emit_bytes(uint8_t b1, uint8_t b2) {
	emit_byte(b1);
	emit_byte(b2);
}

Function* Compiler::end() {
	emit_return();
	Function* func = function;
	//#ifdef DEBUG_PRINT_CODE
	if (!parser.error) {
		current_chunk()->disassemble(func->name != "" ? function->name : "<file>");
	}
	//#endif
	return func;
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
	default:
		break;
	}
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
	emit_constant(make_object(new String(parser.previous_token.value)));
}

void Compiler::declaration() {
	if (match(TOKEN_VAR)) {
		variable_declaration();
	}
	else statement();
	
	if (parser.panic) synchronize();
}

void Compiler::statement() {
	if (match(TOKEN_PRINT)) {
		print_statement();
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
	if (scope_depth > 0) return 0;

	return make_constant(make_object(new String(parser.previous_token.value)));
}

void Compiler::define_variable(uint8_t global) {
	if (scope_depth > 0) {
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
	int arg = resolve_local(name);
	if (arg != -1) {
		get_op = OC_GET_LOCAL;
		set_op = OC_SET_LOCAL;
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
	else {
		emit_bytes(get_op, (uint8_t)arg);
	}
}

uint8_t Compiler::identifier_constant(Token* name) {
	return make_constant(make_object(new String(name->value)));
}

void Compiler::new_scope() {
	scope_depth++;
}
void Compiler::compount_statement() {
	while (parser.current_token.type != TOKEN_R_BRACE && parser.current_token.type != TOKEN_EOF) {
		declaration();
	}

	consume(TOKEN_R_BRACE, "Expected '}'");
}
void Compiler::end_scope() {
	scope_depth--;

	while (local_count > 0 && locals[local_count - 1].depth > scope_depth) {
		emit_byte(OC_POP);
		local_count--;
	}
}

void Compiler::declare_variable() {
	
	if (scope_depth == 0) return;
	Token name = parser.previous_token;
	for (int i = local_count - 1; i >= 0; i--) {
		Local* local = &locals[i];
		if (local->depth != -1 && local->depth < scope_depth) break;

		if (equal_identifiers(name, local->name)) error("Variable already exists in scope");
	}
	
	add_local(name);
}

void Compiler::add_local(Token name) {
	if (local_count == UINT8_MAX + 1) {
		error("Too many local variables in scope");
		return;
	}

	Local* local = &locals[local_count++];
	local->name = name;
	local->depth = -1;
	local->depth = scope_depth;
}

bool Compiler::equal_identifiers(Token& a, Token& b) {
	if (a.value == b.value) return true;
	return false;
}

int Compiler::resolve_local(Token name) {
	for (int i = local_count - 1; i >= 0; i--) {
		Local* local = &locals[i];
		
		if (equal_identifiers(name, local->name)) {
			if (local->depth == -1) error("Tried accessing variable in it's initialization");
			
			return i;
			
		}
	}

	return -1;
}

void Compiler::make_initialized() {
	locals[local_count - 1].depth = scope_depth;
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

int Compiler::emit_jump(uint8_t instruction) {
	emit_byte(instruction);
	emit_byte(0xff);
	emit_byte(0xff);
	return current_chunk()->code.size() - 2;
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
	int loop_start = current_chunk()->code.size();
	expression();

	int exit_jump = emit_jump(OC_JMP_IF_FALSE);
	emit_byte(OC_POP);
	statement();
	emit_loop(loop_start);

	patch_jump(exit_jump);
	emit_byte(OC_POP);
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
	int loop_start = current_chunk()->code.size();
	int exit_jump = -1;
	if (!match(TOKEN_SEMICOLON)) {
		expression();
		consume(TOKEN_SEMICOLON, "Expected ';' after condition");

		exit_jump = emit_jump(OC_JMP_IF_FALSE);
		emit_byte(OC_POP);
	}
	if (!match(TOKEN_DOUBLE_DOT)) {
		int body_jump = emit_jump(OC_JMP);
		int increment_start = current_chunk()->code.size();
		expression();
		consume(TOKEN_DOUBLE_DOT, "Expected ':'");
		emit_byte(OC_POP);

		emit_loop(loop_start);
		loop_start = increment_start;
		patch_jump(body_jump);
	}
	statement();
	emit_loop(loop_start);
	if (exit_jump != -1) {
		patch_jump(exit_jump);
		emit_byte(OC_POP);
	}
	end_scope();
}