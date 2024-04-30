#include "Compiler.h"
#include "Lexer.h"
#include <iostream>
#include <memory>
#include "Value.h"

Compiler::Compiler(const std::string& src, Parser& _parser) : parser(_parser) {
	lexer.source = src;
	rules[TOKEN_L_PAR] = ParseRule{ FN_GROUPING, FN_NONE, PREC_NONE };
	rules[TOKEN_R_PAR] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_L_BRACE] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_R_BRACE] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_COMMA] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_DOT] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
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
	rules[TOKEN_ID] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_STR] = ParseRule{ FN_STRING,     FN_NONE,   PREC_NONE };
	rules[TOKEN_NUM] = ParseRule{ FN_NUMBER,     FN_NONE,   PREC_NONE };
	rules[TOKEN_AND] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_CLASS] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_ELSE] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_FALSE] = ParseRule{ FN_LITERAL,     FN_NONE,   PREC_NONE };
	rules[TOKEN_FOR] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_FUN] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_IF] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
	rules[TOKEN_VOID] = ParseRule{ FN_LITERAL,     FN_NONE,   PREC_NONE };
	rules[TOKEN_OR] = ParseRule{ FN_NONE,     FN_NONE,   PREC_NONE };
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

bool Compiler::compile(Chunk* chunk) {
	compiling_chunk = chunk;
	parser.current_token = Token(TOKEN_PLUS, "", 0);

	while (true) {
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
	lexer.line = 0;
	next();
	expression();
	end();
	return !parser.error;
	
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
	int constant = compiling_chunk->add_constant(value);
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
	
	call_prec_function(prefix_rule);


	while (precedence <= get_rule(parser.current_token.type).precedence) {
		next();
		ParseFn infix_rule = get_rule(parser.previous_token.type).infix;
		call_prec_function(infix_rule);
	}
}

void Compiler::emit_bytes(uint8_t b1, uint8_t b2) {
	emit_byte(b1);
	emit_byte(b2);
}

void Compiler::end() {
	emit_return();
	//#ifdef DEBUG_PRINT_CODE
	if (!parser.error) {
		compiling_chunk->disassemble("code");
	}
	//#endif
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