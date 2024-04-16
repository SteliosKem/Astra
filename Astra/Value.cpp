#include "Value.h"
#include <iostream>

void ValueArray::write(Value value) {
	values.push_back(value);
}

void ValueArray::free() {
	values.clear();
}

void printValue(Value value) {
	std::cout << value;
}