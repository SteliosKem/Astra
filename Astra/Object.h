#pragma once
#include <string>

enum ObjectType {
	
	OBJ_STRING,
	OBJ_FUNCTION,
	OBJ_NATIVE,
	OBJ_CLOSURE,
	OBJ_UPVALUE
};

class Object {
public:
	ObjectType type;
	Object* next;
};

class String : public Object {
public:
	String(std::string src) : str(src) {
		type = OBJ_STRING;
	}
	std::string str;
};
