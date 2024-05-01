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

struct Local {
	Token name;
	int depth;
};

enum ParseFn {
	FN_NONE,
	FN_GROUPING,
	FN_UNARY,
	FN_BINARY,
	FN_NUMBER,
	FN_LITERAL,
	FN_STRING,
	FN_VARIABLE
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

	// HELPER FUNCTIONS
	bool compile(Chunk* chunk);
	void next();
	void consume(TokenType type, const std::string& message);
	bool match(TokenType type);

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


	// EXPRESSIONS, OPERATIONS, LITERALS
	void expression();
	void number();
	void grouping();
	void unary();
	void binary();
	void literal();
	void string();
	
private:
	// ERRORS
	void error_at(Token* token, const std::string& message);
	void error(const std::string& message) {
		error_at(&parser.previous_token, message);
	}
	void synchronize();

	// EMISSIONS
	void emit_return() {
		emit_byte(OC_RETURN);
	}
	void emit_bytes(uint8_t b1, uint8_t b2);

	// STATEMENTS
	void declaration();
	void statement();
	void print_statement();
	void expression_statement();
	void variable_declaration();
	void compount_statement();

	// VARIABLES
	uint8_t parse_variable(const std::string& error);
	void define_variable(uint8_t);
	void variable();
	void named_variable(Token name);
	uint8_t identifier_constant(Token* name);
	bool can_assign;
	void declare_variable();
	void make_initialized();

	// SCOPE
	Local locals[UINT8_MAX + 1];
	int local_count = 0;
	int scope_depth = 0;
	void new_scope();
	void end_scope();
	void add_local(Token name);
	int resolve_local(Token name);
	

	bool equal_identifiers(Token& a, Token& b);
};
