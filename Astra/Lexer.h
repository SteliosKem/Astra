#pragma once
#include <string>
#include <iostream>
#include <memory>

enum TokenType {
	TOKEN_L_PAR, TOKEN_R_PAR, TOKEN_L_BRACE, TOKEN_R_BRACE,
	TOKEN_COMMA, TOKEN_DOT, TOKEN_DOUBLE_DOT, TOKEN_MINUS, TOKEN_PLUS,
	TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR, TOKEN_CAP,

	TOKEN_BANG, TOKEN_BANG_EQUAL,
	TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
	TOKEN_GREATER, TOKEN_GREATER_EQUAL,
	TOKEN_LESS, TOKEN_LESS_EQUAL, TOKEN_PLUS_EQUAL, TOKEN_MINUS_EQUAL,
	TOKEN_STAR_EQUAL, TOKEN_SLASH_EQUAL, TOKEN_PLUS_PLUS, TOKEN_MINUS_MINUS,

	TOKEN_ID, TOKEN_STR, TOKEN_NUM, TOKEN_INTER_STR,

	TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
	TOKEN_FOR, TOKEN_FUN, TOKEN_IF, TOKEN_VOID, TOKEN_OR,
	TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
	TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE, TOKEN_BREAK, TOKEN_CONTINUE,
	TOKEN_RESPOND,

	TOKEN_ERROR, TOKEN_EOF
};

class Token {
public:
	Token(TokenType _type, std::string _value, int _line) : type(_type), value(_value), line(_line) {}
	Token() {}
	Token(const std::string& error, int _line) {
		type = TOKEN_ERROR;
		value = error;
		line = _line;
	}
	TokenType type;
	std::string value;
	int line;
};

class Lexer {
public:
	Lexer(const std::string& _source) : source(_source) {}
	Lexer() { source = ""; }

	char current_char;
	int pos = -1;
	int line = 0;
	std::string source;

	Token lex();

	static bool is_digit(char character);
	static bool is_alpha(char character);
	static TokenType id_type(const std::string& id);		// Decides wether a word is and identifier or keyword
private:
	void next();
	void back();
	bool match(char expected);								// Match next character

	Token str();											// Make string Token
	Token num();											// Make number Token
	Token id();												// Make identifier/keyword Token
};

