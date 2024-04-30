#pragma once
#include <vector>
#include <variant>
#include "Object.h"
#include <memory>

enum ValueType {
	VALUE_BOOL,
	VALUE_VOID,
	VALUE_NUMBER,
	VALUE_OBJECT
};

struct Value {
	ValueType type;
	std::variant<bool, double, Object*> value;
};

class ValueArray {
public:
	std::vector<Value> values;
	void write(Value value);
	void free();
	
};

void print_value(Value value);
void print_object(Value value);

Value make_bool(bool val);
Value make_void();
Value make_number(double val);
Value make_object(Object* val);

bool is_bool(Value val);
bool is_void(Value val);
bool is_number(Value val);
bool is_object(Value val);
bool is_string(Value val);

bool get_bool(Value val);
double get_number(Value val);
Object* get_object(Value val);
String* get_string(Value val);

bool is_object_type(Value value, ObjectType type);