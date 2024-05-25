#pragma once
#include <vector>
#include <variant>
#include <memory>
#include <string>
#include <unordered_map>

enum ObjectType {

	OBJ_STRING,
	OBJ_FUNCTION,
	OBJ_NATIVE,
	OBJ_CLOSURE,
	OBJ_UPVALUE,
	OBJ_CLASS,
	OBJ_INSTANCE,
	OBJ_BOUND_METHOD,
	OBJ_ENUM,
	OBJ_ENUM_VAL,
	OBJ_LIST
};

class Object {
public:
	ObjectType type;
	Object* next;
	bool is_marked = false;
};

class String : public Object {
public:
	String(std::string src) : str(src) {
		type = OBJ_STRING;
	}
	std::string str;
};

class Enumeration : public Object {
public:
	Enumeration() {
		type = OBJ_ENUM;
	}
	Enumeration(std::string _name) {
		name = _name;
		type = OBJ_ENUM;
	}
	std::string name;
	std::vector<std::string> values;
};



class EnumValue : public Object {
public:
	EnumValue(std::string name, Enumeration* _enum) {
		value = name;
		type = OBJ_ENUM_VAL;
		enumeration = _enum;
	}
	std::string value;
	Enumeration* enumeration;
};

enum ValueType {
	VALUE_VOID,
	VALUE_BOOL,
	VALUE_NUMBER,
	VALUE_OBJECT
};

struct Value {
	ValueType type;
	std::variant<bool, double, Object*> value;
};

class ClassObj : public Object {
public:
	ClassObj() {}
	ClassObj(std::string src) : name(src) {
		type = OBJ_CLASS;
	}
	std::string name;
	std::unordered_map<std::string, Value> methods;
};

class Instance : public Object {
public:
	Instance() {
		class_target = nullptr;
		type = OBJ_INSTANCE;
	}
	Instance(ClassObj* _class_target) {
		class_target = _class_target;
		type = OBJ_INSTANCE;
	}
	ClassObj* class_target;
	std::unordered_map<std::string, Value> fields;
};

class List : public Instance {
public:
	List() {
		type = OBJ_LIST;
	}
	std::vector<Value> values;
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
Value make_string(String* val);

bool is_bool(Value val);
bool is_void(Value val);
bool is_number(Value val);
bool is_object(Value val);
bool is_string(Value val);
bool is_class(Value val);
bool is_instance(Value val);
bool is_enum(Value val);
bool is_enum_val(Value val);
bool is_list(Value val);

bool get_bool(Value val);
double get_number(Value val);
Object* get_object(Value val);
String* get_string(Value val);
ClassObj* get_class(Value val);
Instance* get_instance(Value val);
Enumeration* get_enum(Value val);
EnumValue* get_enum_val(Value val);
List* get_list(Value val);

bool is_object_type(Value value, ObjectType type);