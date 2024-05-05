#pragma once
#include <string>

enum ObjectType {
	
	OBJ_STRING,
	OBJ_FUNCTION,
	OBJ_NATIVE
};

class Object {
public:
	ObjectType type;
	Object* next;
};

class String : public Object {
public:
	String(std::string src) : str(src) {}
	std::string str;
};
