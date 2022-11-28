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

	Parser::Parser() {
		value_token.reserve(14);
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
			root = parse_array();
		}
		else {
			save_error(ERROR_SYNTAX, "Document does not start with '{' or '['.");
		}
		
	}

	JSONObject Parser::parse_value() {
		eat_space();

		char ch = peek();

		if (ch == 0) {
			save_error(jacc::ERROR_SYNTAX, "Premature end of documnent while parsing an array.");

			return JSONObject();
		}

		if (ch == '"') {
			return parse_string();
		}
		else if (ch == '{') {
			return parse_object();
		}
		else if (ch == '[') {
			return parse_array();
		}
		else if (isdigit(ch) || ch == '-') {
			return parse_number();
		}
		else if (ch == 't' || ch == 'f') {
			return parse_bool();
		}
		else if (ch == 'n') {
			return parse_null();
		}
		else {
			save_error(jacc::ERROR_SYNTAX, "Unexpected character.");

			return JSONObject();
		}
	}

	void Parser::read_value_token() {
		eat_space();

		value_token.clear();

		while (true) {
			char ch = pop();

			if (ch == 0) {
				save_error(jacc::ERROR_SYNTAX, "Premature end of documnent while parsing a value.");

				return;
			}
			if (ch == '}' || ch == ']' || ch == ',') {
				putback();

				break;
			}
			else {
				value_token.push_back(ch);
			}
		}
	}

	JSONObject Parser::parse_string() {
		return JSONObject();
	}
	JSONObject Parser::parse_object() {
		return JSONObject();
	}

	JSONObject Parser::parse_number() {
		read_value_token();

		if (error_code != jacc::ERROR_NONE) {
			return JSONObject();
		}

		double n = std::stod(value_token);

		return JSONObject(n);
	}

	JSONObject Parser::parse_bool() {
		return JSONObject();
	}
	JSONObject Parser::parse_null() {
		return JSONObject();
	}

	JSONObject Parser::parse_array() {
		eat_space();

		char ch = pop();

		if (ch == 0) {
			save_error(jacc::ERROR_SYNTAX, "Premature end of documnent while parsing an array.");

			return JSONObject();
		}

		if (ch != '[') {
			save_error(jacc::ERROR_SYNTAX, "JSON array does not start with '['.");

			return JSONObject();
		}

		std::vector<JSONObject> list;

		list.reserve(10);

		while ((ch = pop()) != ']') {
			if (ch == 0) {
				save_error(jacc::ERROR_SYNTAX, "Premature end of documnent while parsing an array.");

				return JSONObject();
			}

			putback();

			list.push_back(parse_value());

			eat_space();

			//Next character must be ',' or ']'
			ch = pop();

			if (ch != ',' && ch != ']') {
				save_error(ERROR_SYNTAX, "Invalid character in array.");
				//Stop parsing array
				return JSONObject();
			}
			if (ch != ',') {
				//Next value in array starting
				putback();
			}
		}

		return JSONObject(list);
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