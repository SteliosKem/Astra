#pragma once
#include <string>
#include "Chunk.h"
#include "Lexer.h"
#include <functional>

class Upvalue {
public:
	uint8_t index;
	bool is_local;
};

struct ClassLayer {
	ClassLayer* enclosing;
	bool has_superclass = false;
};

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
	PREC_POWER,
	PREC_CALL,
	PREC_PRIMARY,
	
};

enum FunctionType {
	TYPE_FUNCTION,
	TYPE_SCRIPT,
	TYPE_METHOD,
	TYPE_CONSTRUCTOR
};

struct Local {
	Token name;
	int depth;
	bool is_captured = false;
};

class Layer {
public:
	Layer* enclosing;
	Function* function;
	FunctionType function_type = TYPE_SCRIPT;

	Local locals[UINT8_MAX + 1];
	int local_count = 0;
	std::vector<Upvalue> upvalues;
	int scope_depth = 0;
};

typedef Value(*NativeFn)(int argCount, Value* args);

class Native : public Object {
public:
	Native(NativeFn fn) {
		type = OBJ_NATIVE;
		native_fn = fn;
	}
	NativeFn native_fn;
};

inline bool is_native(Value val) {
	return is_object(val) && get_object(val)->type == OBJ_NATIVE;
}

inline Native* get_native(Value val) {
	return (Native*)get_object(val);
}

enum ParseFn {
	FN_NONE,
	FN_GROUPING,
	FN_UNARY,
	FN_BINARY,
	FN_NUMBER,
	FN_LITERAL,
	FN_STRING,
	FN_VARIABLE,
	FN_AND,
	FN_OR,
	FN_CALL,
	FN_DOT,
	FN_THIS,
	FN_SUPER,
	FN_COMPOUND
};

struct ParseRule {
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
};

struct Parser {
	Parser() {}
	Parser* enclosing = nullptr;
	Token current_token;
	Token previous_token;
	bool error = false;
	bool panic = false;
};


class Compiler {
public:
	ParseRule rules[53];
	Parser& parser;
	Lexer lexer;
	Chunk* current_chunk() {
		return &current->function->chunk;
		//return compiling_chunk;
	}

	Compiler(const std::string& src, Parser& _parser);
	~Compiler() {
		delete current;
	}

	void init_layer(Layer* layer, FunctionType type) {
		layer->enclosing = current;
		layer->function = new Function;
		layer->function_type = type;
		layer->local_count = 0;
		layer->scope_depth = 0;
		current = layer;

		if (type != TYPE_SCRIPT) {
			current->function->name = parser.previous_token.value;
		}

		Local* local = &current->locals[current->local_count++];
		local->depth = 0;
		local->name.value = "";
		local->is_captured = false;
		if (type != TYPE_FUNCTION)
			local->name.value = "this";
		else
			local->name.value = "";
	}

	// HELPER FUNCTIONS
	Function* compile();
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
		current_chunk()->write(byte, parser.previous_token.line);
	}

	uint8_t make_constant(Value value);
	Function* end();


	// EXPRESSIONS, OPERATIONS, LITERALS
	void expression();
	void number();
	void grouping();
	void unary();
	void binary();
	void literal();
	void string();
	void mark_compiler_roots();
private:
	// ERRORS
	void error_at(Token* token, const std::string& message);
	void error(const std::string& message) {
		error_at(&parser.previous_token, message);
	}
	void synchronize();

	// EMISSIONS
	void emit_return() {
		if (current->function_type == TYPE_CONSTRUCTOR) {
			emit_bytes(OC_GET_LOCAL, 0);
		}
		else {
			emit_byte(OC_VOID);
		}
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
	void if_statement();
	void respond_statement();

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
	Layer* current;
	void new_scope();
	void end_scope();
	void add_local(Layer* layer, Token& name);
	int resolve_local(Layer* layer, Token& name);

	// IF
	int emit_jump(uint8_t instruction, int pos = -1);
	void patch_jump(int offset);
	

	bool equal_identifiers(Token& a, Token& b);

	// LOGICAL OPERATIONS
	void and_operation();
	void or_operation();

	// LOOPS
	void while_statement();
	void for_statement();
	void emit_loop(int loop_start);
	int current_exit_jump = 0;
	int current_loop_start = -1;
	int current_loop_scope_depth = 0;
	std::vector<int> break_pos;
	std::vector<int> continue_pos;
	void break_statement();
	void continue_statement();

	// FUNCTIONS
	void function_declaration();
	void make_function(FunctionType type);
	void function_call();
	uint8_t argument_list();
	void return_statement();
	int resolve_upvalue(Layer* layer, Token& name);
	int add_upvalue(Layer* layer, uint8_t index, bool is_local);

	// OOP
	void class_declaration();
	void access();
	void parse_method();
	void this_oop();
	ClassLayer* current_class = nullptr;
	void parse_super();

	void expression_compound();
	std::vector<int> respond_jumps;

	void enum_declaration();
};
