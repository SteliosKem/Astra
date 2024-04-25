#pragma once
#include <vector>
#include <variant>


enum ValueType {
	VALUE_BOOL,
	VALUE_VOID,
	VALUE_NUMBER
};

struct Value {
	ValueType type;
	std::variant<bool, double> value;
};


class ValueArray {
public:
	std::vector<Value> values;
	void write(Value value);
	void free();
	
};

void print_value(Value value);

Value make_bool(bool val);
Value make_void();
Value make_number(double val);

bool is_bool(Value val);
bool is_void(Value val);
bool is_number(Value val);

bool get_bool(Value val);
double get_number(Value val);