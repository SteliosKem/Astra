#pragma once
#include <string>
#include "Chunk.h"
#include "Lexer.h"
#include <functional>

enum Precedence {
	PREC_NONE,
	PREC_ASSIGNMENT,
	PREC_OR,
	PREC_AND,
	PREC_EQUALITY,
	PREC_COMPARISON,
	PREC_TERM,
	PREC_FACTOR,
	PREC_UNARY,
	PREC_CALL,
	PREC_PRIMARY
};

enum ParseFn {
	FN_NONE,
	FN_GROUPING,
	FN_UNARY,
	FN_BINARY,
	FN_NUMBER,
	FN_LITERAL
};

struct ParseRule {
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
};

struct Parser {
	Parser() {}
	Token current_token;
	Token previous_token;
	bool error = false;
	bool panic = false;
};


class Compiler {
public:
	ParseRule rules[40];
	Parser& parser;
	Lexer lexer;
	Chunk* compiling_chunk;

	Compiler(const std::string& src, Parser& _parser);
	bool compile(Chunk* chunk);
	void next();
	void consume(TokenType type, const std::string& message);

	void expression();
	void number();
	void grouping();
	void unary();
	void binary();
	void literal();
	void parse_precedence(Precedence precedence);

	ParseRule get_rule(TokenType type) {
		return rules[type];
	}

	void call_prec_function(ParseFn func);

	void emit_constant(Value value) {
		emit_bytes(OC_CONSTANT, make_constant(value));
	}

	void emit_byte(uint8_t byte) {
		compiling_chunk->write(byte, parser.previous_token.line);
	}

	uint8_t make_constant(Value value);
	void end();
private:
	void error_at(Token* token, const std::string& message);
	void error(const std::string& message) {
		error_at(&parser.previous_token, message);
	}
	void emit_return() {
		emit_byte(OC_RETURN);
	}
	void emit_bytes(uint8_t b1, uint8_t b2);
};