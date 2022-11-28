#include "Parser.h"
#include <iostream>

namespace jacc {
	JSONObject::JSONObject() {

	}

	JSONObject::JSONObject(const std::string_view& s) : str(s), type(JSON_STRING) {

	}

	JSONObject::JSONObject(std::map<std::string_view, JSONObject>& o) : object(std::move(o)), type(JSON_OBJECT) {

	}

	JSONObject::JSONObject(std::vector<JSONObject>& a) : array(std::move(a)), type(JSON_ARRAY) {

	}

	JSONObject::JSONObject(double n) : number(n), type(JSON_NUMBER) {

	}

	JSONObject::JSONObject(bool b) : booleanValue(b), type(JSON_BOOLEAN) {

	}


	JSONObject::JSONObject(JSONObject&& other) {
		type = other.type;

		if (other.type == JSON_STRING) {
			str = other.str;
		}
		else if (other.type == JSON_NUMBER) {
			number = other.number;
		}
		else if (other.type == JSON_ARRAY) {
			array = std::move(other.array);
		}
		else if (other.type == JSON_OBJECT) {
			object = std::move(other.object);
		}
		else if (other.type == JSON_BOOLEAN) {
			booleanValue = other.booleanValue;
		}

		other.type = JSON_UNDEFINED;
	}

	JSONObject& JSONObject::operator=(JSONObject&& other) {
		if (this != &other) {
			type = other.type;

			if (other.type == JSON_STRING) {
				str = other.str;
			}
			else if (other.type == JSON_NUMBER) {
				number = other.number;
			}
			else if (other.type == JSON_ARRAY) {
				array = std::move(other.array);
			}
			else if (other.type == JSON_OBJECT) {
				object = std::move(other.object);
			}
			else if (other.type == JSON_BOOLEAN) {
				booleanValue = other.booleanValue;
			}

			other.type = JSON_UNDEFINED;
		}

		return *this;
	}

	void Parser::parse(std::string_view source) {
		data = source;
		location = 0;

		eat_space();

		char ch = peek();

		if (ch == '{') {
			//parser->root = parseObject(parser);
		}
		else if (ch == '[') {

		}
		else {
			save_error(ERROR_SYNTAX, "Document does not start with '{' or '['.");
		}
		
	}

	char Parser::peek() {
		if (location < data.size()) {
			return data.at(location);
		}
		else {
			return '\0';
		}
	}
	char Parser::pop() {
		if (location < data.size()) {
			char result = data.at(location);

			++location;

			return result;
		}
		else {
			return '\0';
		}
	}

	void Parser::putback() {
		if (location > 0) {
			--location;
		}
	}

	void Parser::eat_space() {
		char ch = pop();

		if (ch == 0) return;

		while (isspace(ch)) {
			ch = pop();

			if (ch == 0) return;
		}

		putback();
	}

	void Parser::save_error(ErrorCode code, const char* msg) {
		error_code = code;
		error_message = msg;
	}
}