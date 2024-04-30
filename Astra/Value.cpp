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
		if (get_bool(value)) std::cout << "true";
		else std::cout << "false";
		break;
	case VALUE_VOID:
		std::cout << "void";
		break;
	case VALUE_NUMBER:
		std::cout << get_number(value);
		break;
	case VALUE_OBJECT:
		print_object(value);
		break;
	}
	
}

void print_object(Value value) {
	switch (get_object(value)->type) {
	case OBJ_STRING:
		std::cout << get_string(value)->str;
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

Value make_object(Object* obj) {
	return Value(VALUE_OBJECT, obj);
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
bool is_object(Value val) {
	return val.type == VALUE_OBJECT;
}
bool is_string(Value val) {
	return val.type == VALUE_OBJECT && std::get<Object*>(val.value)->type == OBJ_STRING;
}

double get_number(Value val) {
	return std::get<double>(val.value);
}
bool get_bool(Value val) {
	return std::get<bool>(val.value);
}
Object* get_object(Value val) {
	return std::get<Object*>(val.value);
}
String* get_string(Value val) {
	return (String*)get_object(val);
}

bool is_object_type(Value value, ObjectType type) {
	return is_object(value) && get_object(value)->type == type;
}