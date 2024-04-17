#include "Compiler.h"
#include "Lexer.h"
#include <iostream>

void compile(const std::string& source) {
	Lexer lexer(source);
	while (true) {
		Token token = lexer.lex();
		if (token.line != lexer.line) {
			std::cout << "    " << token.line;
			lexer.line = token.line;
		}
		else
			std::cout << "    | ";
		std::cout << token.type << " " << token.value << std::endl;
		if (token.type == _EOF) break;
	}
}