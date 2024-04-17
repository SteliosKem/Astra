#include "Lexer.h"
#include <iostream>
#include <string>

Token Lexer::lex() {
	next();

    while (current_char == ' ') {
        /*if (current_char == '\n')
            line++;
        else if (current_char == '/') {
            if (!at_end() && source[current + 1] == '/') {
                while (current_char != '\n' && !at_end())
                    current_char = next();
            }
            else
                break;
        }*/
        next();
    }
    if (current_char == '\0') return Token(_EOF, "", line);
    if (is_digit(current_char)) return num();
    if (is_alpha(current_char)) return id();

	switch (current_char) {
        case '(': return Token(L_PAR, "", line);
        case ')': return Token(R_PAR, "", line);
        case '{': return Token(L_BRACE, "", line);
        case '}': return Token(R_BRACE, "", line);
        case ';': return Token(SEMICOLON, "", line);
        case ',': return Token(COMMA, "", line);
        case '.': return Token(DOT, "", line);
        case '-': return Token(MINUS, "", line);
        case '+': return Token(PLUS, "", line);
        case '/': return Token(SLASH, "", line);
        case '*': return Token(STAR, "", line);
        case '!':
            return Token(
                match('=') ? BANG_EQUAL : BANG, "", line);
        case '=':
            return Token(
                match('=') ? EQUAL_EQUAL : EQUAL, "", line);
        case '<':
            return Token(
                match('=') ? LESS_EQUAL : LESS, "", line);
        case '>':
            return Token(
                match('=') ? GREATER_EQUAL : GREATER, "", line);
        case '"':
            return str();
	}
    return Token("Unexpected character: " + current_char, line);
}

Token Lexer::str() {
    std::string string;
    next();
    while (current_char != '"' && current_char != '\0') {
        if (current_char == '\n')
            line++;
        string += current_char;
        next();
    }

    if (current_char == '\0')
        return Token("Unterminated string", line);
    return Token(TokenType::STR, string, line);
}

Token Lexer::num() {
    std::string num_string;
    bool is_float = false;
    bool has_error = false;

    while (is_digit(current_char)) {
        if (current_char == '.') {
            if (is_float)
                has_error = true;
            is_float = true;
        }
        num_string += current_char;
        next();
    }
    back();

    if (has_error)
        return Token("Unexpected dot", line);

    return Token(TokenType::NUM, num_string, line);
}

Token Lexer::id() {
    std::string string;

    while (is_alpha(current_char) || is_digit(current_char)) {
        string += current_char;
        next();
    }
    back();
    return Token(id_type(string), string, line);
}