#include "Value.h"
#include <iostream>

void ValueArray::write(Value value) {
	values.push_back(value);
}

void ValueArray::free() {
	values.clear();
}

void print_value(Value value) {
	switch (value.type) {
	case VALUE_BOOL:
		std::cout << get_bool(value) ? "true" : "false";
		break;
	case VALUE_VOID:
		std::cout << "void";
		break;
	case VALUE_NUMBER:
		std::cout << get_number(value);
		break;
	}
	
}

Value make_bool(bool val) {
	return Value(VALUE_BOOL, val);
}

Value make_void() {
	return Value(VALUE_VOID);
}

Value make_number(double val) {
	return Value(VALUE_NUMBER, val);
}

bool is_bool(Value val) {
	return val.type == VALUE_BOOL;
}
bool is_void(Value val) {
	return val.type == VALUE_VOID;
}
bool is_number(Value val) {
	return val.type == VALUE_NUMBER;
}

double get_number(Value val) {
	return std::get<double>(val.value);
}
bool get_bool(Value val) {
	return std::get<bool>(val.value);
}