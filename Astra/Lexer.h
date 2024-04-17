#pragma once
#include <string>
#include <iostream>

enum TokenType {
	L_PAR, R_PAR, L_BRACE, R_BRACE,
	COMMA, DOT, MINUS, PLUS,
	SEMICOLON, SLASH, STAR,

	BANG, BANG_EQUAL,
	EQUAL, EQUAL_EQUAL,
	GREATER, GREATER_EQUAL,
	LESS, LESS_EQUAL,

	ID, STR, NUM,

	AND, CLASS, ELSE, FALSE,
	FOR, FUN, IF, VOID, OR,
	PRINT, RETURN, SUPER, THIS,
	TRUE, VAR, WHILE,

	ERROR, _EOF
};

class Token {
public:
	Token(TokenType _type, std::string _value, int _line) : type(_type), value(_value), line(_line) {}
	Token(const std::string& error, int _line) {
		type = ERROR;
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
	char current_char;
	int pos = -1;
	int line = 0;
	const std::string& source;
	Token lex();

	static bool is_digit(char character) {
		return character >= '0' && character <= '9';
	}

	static bool is_alpha(char character) {
		return (character >= 'a' && character <= 'z') ||
			(character >= 'A' && character <= 'Z') || character == '_';
	}

	static TokenType id_type(const std::string& id) {
		switch (id[0]) {
		case 'a': {
			if (id == "and") return AND;
			break;
		}
		case 'c': {
			if (id == "class") return CLASS;
			break;
		}
		case 'e': {
			if (id == "else") return ELSE;
			break;
		}
		case 'i': {
			if (id == "if") return IF;
			break;
		}
		case 'v': {
			if (id == "void") return VOID;
			break;
		}
		case 'o': {
			if (id == "or") return OR;
			break;
		}
		case 'p': {
			if (id == "print") return PRINT;
			break;
		}
		case 'r': {
			if (id == "return") return AND;
			break;
		}
		case 's': {
			if (id == "super") return SUPER;
			break;
		}
		case 'l': {
			if (id == "let") return VAR;
			break;
		}
		case 'w': {
			if (id == "while") return WHILE;
			break;
		}
		case 'f': {
			if (id == "false") return FALSE;
			else if (id == "for") return FOR;
			else if (id == "fn") return FUN;
			break;
		}
		case 't': {
			if (id == "this") return THIS;
			else if (id == "true") return TRUE;
			break;
		}
		}
		return ID;
	}
private:
	void next() {
		pos++;
		current_char = pos < source.size() ? source[pos] : '\0';
	}
	void back() {
		pos--;
		current_char = source[pos];
	}
	bool match(char expected) {
		if (pos + 1 >= source.size() || source[pos + 1] != expected) return false;
		next();
		return true;
	}

	Token str();
	Token num();
	Token id();
};

