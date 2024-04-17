#include "Value.h"
#include <iostream>

void ValueArray::write(Value value) {
	values.push_back(value);
}

void ValueArray::free() {
	values.clear();
}

void print_value(Value value) {
	std::cout << value;
}