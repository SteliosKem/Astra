#include "Lexer.h"
#include <iostream>
#include <string>

Token Lexer::lex() {
	next();

    while (current_char == ' ' || current_char == '\n' || current_char == '\t' || current_char == '/') {                                   // Skip whitespace
        if (current_char == '\n')                                 // Change line on newline character
            line++;
        else if (current_char == '/') {                             // Skip comments
            if (pos < source.size() && source[pos + 1] == '/') {
                while (current_char != '\n' && pos < source.size())
                    next();
            }
            else if (pos < source.size() && source[pos + 1] == '*') {
                next();
                next();
                while (pos < source.size()) {
                    if (current_char == '\n') {
                        line++;
                        next();
                    }
                    else if (current_char == '*' && source[pos + 1] == '/') {
                        next();
                        next();
                        break;
                    }
                    else {
                        next();
                    }
                }
            }
            else
                break;
        }
        next();
    }
    if (current_char == '\0') return Token(TOKEN_EOF, "", line);    // If at end return EOF
    if (is_digit(current_char)) return num();                       // Make number token
    if (is_alpha(current_char)) return id();                        // Make identifier/keyword token

	switch (current_char) {                                         // Make single/double character tokens or string token
        case '(': return Token(TOKEN_L_PAR, "", line);
        case ')': return Token(TOKEN_R_PAR, "", line);
        case '{': return Token(TOKEN_L_BRACE, "", line);
        case '}': return Token(TOKEN_R_BRACE, "", line);
        case ';': return Token(TOKEN_SEMICOLON, "", line);
        case ',': return Token(TOKEN_COMMA, "", line);
        case '.': return Token(TOKEN_DOT, "", line);
        case '^': return Token(TOKEN_CAP, "", line);
        case '-': return Token(
            match('=') ? TOKEN_MINUS_EQUAL : TOKEN_MINUS, "", line);
        case '+': return Token(
            match('=') ? TOKEN_PLUS_EQUAL : TOKEN_PLUS, "", line);
        case '/': return Token(
            match('=') ? TOKEN_SLASH_EQUAL : TOKEN_SLASH, "", line);
        case '*': return Token(
            match('=') ? TOKEN_STAR_EQUAL : TOKEN_STAR, "", line);
        case ':': return Token(TOKEN_DOUBLE_DOT, "", line);
        case '!':
            return Token(
                match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG, "", line);
        case '=':
            return Token(
                match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL, "", line);
        case '<':
            return Token(
                match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS, "", line);
        case '>':
            return Token(
                match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER, "", line);
        case '"':
            return str();
	}
    return Token("Unexpected character: " + current_char, line);    // If nothing matches return error
}

Token Lexer::str() {
    std::string string;
    bool interpolated = false;
    next();
    while (current_char != '"' && current_char != '\0') {           // Make the string body
        if (current_char == '[' && source[pos + 1] == '{')
            interpolated = true;
        if (current_char == '\n')
            line++;
        string += current_char;
        next();
    }

    if (current_char == '\0')
        return Token("Unterminated string", line);                  // If the lexer reaches the end without closing the string return an error
    if (interpolated)
        return Token(TOKEN_INTER_STR, string, line);
    return Token(TOKEN_STR, string, line);
}

Token Lexer::num() {
    std::string num_string;
    bool is_float = false;
    bool has_error = false;

    while (is_digit(current_char) || current_char == '.') {         // Make number body
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

    return Token(TOKEN_NUM, num_string, line);
}

Token Lexer::id() {
    std::string string;

    while (is_alpha(current_char) || is_digit(current_char)) {      // Make identifier body
        string += current_char;
        next();
    }
    back();
    return Token(id_type(string), string, line);                    // Check if name is identifier or keyword
}

bool Lexer::match(char expected) {
    if (pos + 1 >= source.size() || source[pos + 1] != expected) return false;
    next();
    return true;
}

void Lexer::back() {
    pos--;
    current_char = source[pos];
}

void Lexer::next() {
    pos++;
    current_char = pos < source.size() ? source[pos] : '\0';
}

TokenType Lexer::id_type(const std::string& id) {
    switch (id[0]) {
        case 'a': {
            if (id == "and") return TOKEN_AND;
            break;
        }
        case 'b': {
            if (id == "break") return TOKEN_BREAK;
            break;
        }
        case 'c': {
            if (id == "class") return TOKEN_CLASS;
            else if (id == "continue") return TOKEN_CONTINUE;
            break;
        }
        case 'e': {
            if (id == "else") return TOKEN_ELSE;
            break;
        }
        case 'i': {
            if (id == "if") return TOKEN_IF;
            break;
        }
        case 'v': {
            if (id == "void") return TOKEN_VOID;
            break;
        }
        case 'o': {
            if (id == "or") return TOKEN_OR;
            break;
        }
        case 'p': {
            if (id == "print") return TOKEN_PRINT;
            break;
        }
        case 'r': {
            if (id == "return") return TOKEN_RETURN;
            break;
        }
        case 's': {
            if (id == "super") return TOKEN_SUPER;
            break;
        }
        case 'l': {
            if (id == "let") return TOKEN_VAR;
            break;
        }
        case 'w': {
            if (id == "while") return TOKEN_WHILE;
            break;
        }
        case 'f': {
            if (id == "false") return TOKEN_FALSE;
            else if (id == "for") return TOKEN_FOR;
            else if (id == "fn") return TOKEN_FUN;
            break;
        }
        case 't': {
            if (id == "this") return TOKEN_THIS;
            else if (id == "true") return TOKEN_TRUE;
            break;
        }
    }
    return TOKEN_ID;
}

bool Lexer::is_alpha(char character) {
    return (character >= 'a' && character <= 'z') ||
        (character >= 'A' && character <= 'Z') || character == '_';
}

bool Lexer::is_digit(char character) {
    return character >= '0' && character <= '9';
}