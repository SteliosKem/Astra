#pragma once
#include "Value.h"
class Memory {
public:
	
	static void mark_value(Value value);
	static void mark_object(Object* object);
	static void mark_table(std::unordered_map<std::string, Value>& map);
};