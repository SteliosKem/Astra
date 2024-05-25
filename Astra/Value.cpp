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

Value make_string(String* val) {
	val->type = OBJ_STRING;
	return make_object(val);
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
bool is_class(Value val) {
	return val.type == VALUE_OBJECT && std::get<Object*>(val.value)->type == OBJ_CLASS;
}
bool is_instance(Value val) {
	return val.type == VALUE_OBJECT && std::get<Object*>(val.value)->type == OBJ_INSTANCE;
}
bool is_enum(Value val) {
	return val.type == VALUE_OBJECT && std::get<Object*>(val.value)->type == OBJ_ENUM;
}
bool is_enum_val(Value val) {
	return val.type == VALUE_OBJECT && std::get<Object*>(val.value)->type == OBJ_ENUM_VAL;
}
bool is_list(Value val) {
	return val.type == VALUE_OBJECT && std::get<Object*>(val.value)->type == OBJ_LIST;
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
ClassObj* get_class(Value val) {
	return (ClassObj*)get_object(val);
}
Instance* get_instance(Value val) {
	return (Instance*)get_object(val);
}
Enumeration* get_enum(Value val) {
	return (Enumeration*)get_object(val);
}
EnumValue* get_enum_val(Value val) {
	return (EnumValue*)get_object(val);
}
List* get_list(Value val) {
	return (List*)get_object(val);
}


bool is_object_type(Value value, ObjectType type) {
	return is_object(value) && get_object(value)->type == type;
}