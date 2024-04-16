#pragma once
#include <vector>
typedef double Value;

class ValueArray {
public:
	std::vector<Value> values;
	void write(Value value);
	void free();
	
};

void printValue(Value value);