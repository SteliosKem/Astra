#pragma once
#include <string>

enum ObjectType {
	OBJ_STRING,
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
