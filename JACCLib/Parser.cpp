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
	 * Code stolen from: http://stackoverflow.com/a/4609989/1036017
	 */
	static void unicode_to_UTF8(int unicode, char* out, int* bytes_written) {
		char* pos = out;

		if (unicode < 0x80) *pos++ = unicode;
		else if (unicode < 0x800) *pos++ = 192 + unicode / 64, * pos++ = 128 + unicode % 64;
		else if (unicode - 0xd800u < 0x800) {}
		else if (unicode < 0x10000) *pos++ = 224 + unicode / 4096, * pos++ = 128 + unicode / 64 % 64, * pos++ = 128 + unicode % 64;
		else if (unicode < 0x110000) *pos++ = 240 + unicode / 262144, * pos++ = 128 + unicode / 4096 % 64, * pos++ = 128 + unicode / 64 % 64, * pos++ = 128 + unicode % 64;


		*bytes_written = (pos - out);
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
					char in[5];

					in[0] = pop();
					in[1] = pop();
					in[2] = pop();
					in[3] = pop();
					in[4] = '\0';

					int unicode;

					sscanf(in, "%04X", &unicode);

					char out[4];
					int bytes_written;

					unicode_to_UTF8(unicode, out, &bytes_written);

					if (bytes_written == 0) {
						save_error(ERROR_SYNTAX, "Failed to convert UNICODE to UTF-8.");

						return;
					}

					for (int i = 0; i < bytes_written; ++i) {
						s.push_back(out[i]);
					}

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
