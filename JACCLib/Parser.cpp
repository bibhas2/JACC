#include "Parser.h"
#include <iostream>

namespace jacc {
	JSONObject::JSONObject() {

	}

	JSONObject::JSONObject(std::string& s) : str(std::move(s)), type(JSON_STRING) {

	}
	JSONObject::JSONObject(const char *s) : str(s), type(JSON_STRING) {

	}
	JSONObject::JSONObject(std::map<std::string, JSONObject>& o) : object(std::move(o)), type(JSON_OBJECT) {

	}

	JSONObject::JSONObject(std::vector<JSONObject>& a) : array(std::move(a)), type(JSON_ARRAY) {

	}

	JSONObject::JSONObject(double n) : number(n), type(JSON_NUMBER) {

	}

	JSONObject::JSONObject(bool b) : booleanValue(b), type(JSON_BOOLEAN) {

	}


	JSONObject::JSONObject(JSONObject&& other) noexcept {
		type = other.type;

		if (other.type == JSON_STRING) {
			str = std::move(other.str);
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

	JSONObject& JSONObject::operator=(JSONObject&& other) noexcept {
		if (this != &other) {
			type = other.type;

			if (other.type == JSON_STRING) {
				str = std::move(other.str);
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

	/*
	* According to https://www.ietf.org/rfc/rfc4627.txt, code points larger
	* than 0xFFFF must be encoded in a JSON string using UTF-16. This routine
	* doesn't handle that situation. For example, U+1D11E may be represented as
	* "\uD834\uDD1E"
	*/
	void utf8_encode(std::string& str, unsigned long code_point) {
		if (code_point <= 0x007F) {
			char ch = static_cast<char>(code_point);

			str.push_back(ch);
		}
		else if (code_point <= 0x07FF) {
			uint8_t b2 = 0b10000000 | (code_point & 0b111111);
			uint8_t b1 = 0b11000000 | (code_point >> 6);

			str.push_back(b1);
			str.push_back(b2);
		}
		else if (code_point <= 0xFFFF) {
			uint8_t b3 = 0b10000000 | (code_point & 0b111111);
			uint8_t b2 = 0b10000000 | ((code_point >> 6) & 0b111111);
			uint8_t b1 = 0b11100000 | (code_point >> 12);

			str.push_back(b1);
			str.push_back(b2);
			str.push_back(b3);
		}
		else if (code_point <= 0x10FFFF) {
			uint8_t b4 = 0b10000000 | (code_point & 0b111111);
			uint8_t b3 = 0b10000000 | ((code_point >> 6) & 0b111111);
			uint8_t b2 = 0b10000000 | ((code_point >> 12) & 0b111111);
			uint8_t b1 = 0b11110000 | (code_point >> 18);

			str.push_back(b1);
			str.push_back(b2);
			str.push_back(b3);
			str.push_back(b4);
		}
	}

	Parser::Parser(Reader& r) : reader(r) {
		value_token.reserve(14);
	}

	void Parser::parse() {
		eat_space();

		char ch = peek();

		if (ch == '{') {
			root = parse_object();
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
		std::string s;

		s.reserve(25);

		read_quoted_string(s);

		if (error_code != jacc::ERROR_NONE) {
			return JSONObject();
		}
		else {
			return JSONObject(s);
		}
	}

	void Parser::read_quoted_string(std::string& s) {
		eat_space();

		char ch = pop();

		if (ch == 0) {
			save_error(ERROR_SYNTAX, "Premature end of document while parsing string.");

			return;
		}
		else if (ch != '"') {
			save_error(ERROR_SYNTAX, "String does not start with \".");

			return;
		}

		s.clear();

		while ((ch = pop()) != '"') {
			if (ch == 0) {
				save_error(ERROR_SYNTAX, "Premature end of document while parsing string.");

				return;
			}

			if (ch == '\\') {
				//Escape handling

				char escaped = pop();

				if (escaped == 0) {
					save_error(ERROR_SYNTAX, "Invalid escaped character in string.");

					return;
				}

				if (escaped == 't') {
					ch = '\t';
				}
				else if (escaped == 'r') {
					ch = '\r';
				}
				else if (escaped == 'n') {
					ch = '\n';
				}
				else if (escaped == 'b') {
					ch = '\b';
				}
				else if (escaped == '"') {
					ch = '"';
				}
				else if (escaped == '\\') {
					ch = '\\';
				}
				else if (escaped == 'u') {
					//Unicode escape. Must be 2 hex digits.
					char buff[5];

					buff[0] = pop();
					buff[1] = pop();
					buff[2] = pop();
					buff[3] = pop();
					buff[4] = '\0';

					if (::strlen(buff) != 4) {
						save_error(ERROR_SYNTAX, "Invalid Unicode in string.");

						return;
					}

					unsigned long code_point = std::strtoul(buff, nullptr, 16);

					utf8_encode(s, code_point);

					continue;
				}
			}

			s.push_back(ch);
		}
	}

	JSONObject Parser::parse_object() {
		char ch = pop();

		if (ch == 0) {
			save_error(ERROR_SYNTAX, "Premature end of document while parsing string.");

			return JSONObject();
		}
		if (ch != '{') {
			save_error(ERROR_SYNTAX, "Object does not start with '{'.");

			return JSONObject();
		}

		std::map<std::string, JSONObject> map;
		std::string name;

		name.reserve(25);

		while (true) {
			eat_space();
			ch = pop();

			if (ch == 0) {
				save_error(ERROR_SYNTAX, "Premature end of document while parsing an object.");
				
				break;
			}
			else if (ch == '}') {
				//End of object
				break;
			}
			else if (ch == '"') {
				putback();
				read_quoted_string(name);

				if (error_code != jacc::ERROR_NONE) {
					return JSONObject();
				}
			}
			else if (ch == ':') {
				map.emplace(name, parse_value());

				if (error_code != jacc::ERROR_NONE) {
					return JSONObject();
				}
			}
			else if (ch == ',') {
				//End of a property. Nothing to do here.
			}
			else {
				save_error(ERROR_SYNTAX, "Invalid character in an object.");
			}

			if (error_code != ERROR_NONE) {
				return JSONObject();
			}
		}

		return JSONObject(map);
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
		read_value_token();

		if (error_code != jacc::ERROR_NONE) {
			return JSONObject();
		}

		if (value_token == "true") {
			return JSONObject(true);
		} 
		else if (value_token == "false") {
			return JSONObject(false);
		}
		else {
			save_error(ERROR_SYNTAX, "Invalid boolean value.");

			return JSONObject();
		}
	}

	JSONObject Parser::parse_null() {
		read_value_token();

		if (error_code != jacc::ERROR_NONE) {
			return JSONObject();
		}

		if (value_token == "null") {
			JSONObject o;

			o.type = jacc::JSON_NULL;

			return o;
		}
		else {
			save_error(ERROR_SYNTAX, "Invalid null value.");

			return JSONObject();
		}
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
		return reader.peek();
	}
	char Parser::pop() {
		return reader.pop();
	}

	void Parser::putback() {
		reader.putback();
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
