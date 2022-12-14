#include "Parser.h"
#include <iostream>

namespace jacc {
	JSONObject::JSONObject() : value(jacc::JSON_UNDEFINED()) {
	}
    JSONObject::JSONObject(jacc::JSON_NULL n) : value(n) {
    }
	JSONObject::JSONObject(std::string& s) : value(std::move(s)) {
	}

	JSONObject::JSONObject(const char *s) : value(std::string(s)) {
	}

	JSONObject::JSONObject(std::map<std::string, JSONObject>& o) : value(std::move(o)) {
	}

	JSONObject::JSONObject(std::vector<JSONObject>& a) : value(std::move(a)) {
	}

	JSONObject::JSONObject(double n) : value(n) {
	}

	JSONObject::JSONObject(bool b) : value(b) {
	}

	JSONObject::JSONObject(JSONObject&& other) noexcept : value(std::move(other.value))  {
        other.value = jacc::JSON_UNDEFINED();
	}

	JSONObject& JSONObject::operator=(JSONObject&& other) noexcept {
		if (this != &other) {
            value = std::move(other.value);
            other.value = jacc::JSON_UNDEFINED();
		}

		return *this;
	}

    bool JSONObject::isUndefined() {
        return std::holds_alternative<jacc::JSON_UNDEFINED>(value);
    }

    bool JSONObject::isNull() {
        return std::holds_alternative<jacc::JSON_NULL>(value);
    }

    bool JSONObject::isString() {
        return std::holds_alternative<std::string>(value);
    }

    bool JSONObject::isNumber() {
        return std::holds_alternative<double>(value);
    }

    bool JSONObject::isObject() {
        return std::holds_alternative<std::map<std::string, JSONObject>>(value);
    }

    bool JSONObject::isArray() {
        return std::holds_alternative<std::vector<JSONObject>>(value);
    }

    bool JSONObject::isBoolean() {
        return std::holds_alternative<bool>(value);
    }

    std::string& JSONObject::string() {
        return std::get<std::string>(value);
    }

    double JSONObject::number() {
        return std::get<double>(value);
    }

    std::map<std::string, JSONObject>& JSONObject::object() {
        return std::get<std::map<std::string, JSONObject>>(value);
    }

    std::vector<JSONObject>& JSONObject::array() {
        return std::get<std::vector<JSONObject>>(value);
    }

    bool JSONObject::boolean() {
        return std::get<bool>(value);
    }

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

	JSONObject Parser::parse() {
		eat_space();

		char ch = peek();

		if (ch == '{') {
			return parse_object();
		}
		else if (ch == '[') {
			return parse_array();
		}
		else {
			save_error(ERROR_SYNTAX, "Document does not start with '{' or '['.");
		}
		
        return JSONObject();
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
                    /*
                     JSON Unicode escape basics:
                     
                     Code points U+FFFF and below are supplied in JSON as is without any kind of encoding.
                     The escape must use 4 hex digits after a \u.
                     Example: code point U+03A9 is escpaed as "\u03A9".
                     
                     A code point above U+FFFF is first UTF-16 encoded. This produces two 16 bit integers (called surrogate pairs).
                     They are then supplied in JSON as two consecutive 4 hex digit integers.
                     Example: code point U+1D11E is escaped in JSON as "\uD834\uDD1E".
                     
                     The first integer in the pair is always in range of (0xD800, 0xDFFF) inclusive.
                     No valid code point exists in that range. So an integer in that range
                     signals that this is a leading integer in a UTF-16 encoding.
                     
                     See Section 2.5: https://www.ietf.org/rfc/rfc4627.txt
                     */
                    
                    uint16_t i1 = read_codepoint();

					if (error_code != ERROR_NONE) {
						return;
					}
                    
                    if (i1 >= 0xD800 && i1 <= 0xDFFF) {
                        //We need to read the next 16 bit
                        if (pop() != '\\' || pop() != 'u') {
                            save_error(ERROR_SYNTAX, "Unicode code point above U+FFFF not escaped correctly.");
                            
                            return;
                        } else {
                            uint16_t i2 = read_codepoint();
                            
                            if (error_code != ERROR_NONE) {
                                return;
                            }
                            
                            unsigned long code_point = decode_utf16(i1, i2);

                            if (error_code != ERROR_NONE) {
                                return;
                            }
                            
                            utf8_encode(s, code_point);
                        }
                    } else {
                        //Code point is given as is. No need to decode.
                        utf8_encode(s, i1);
                    }

					continue;
				}
			}

			s.push_back(ch);
		}
	}

    /*
     Convert a UTF-16 encoded surrogate pair to code point.
     Wikipedia does a great job explaing the UTF-16 encoding scheme.
     https://en.wikipedia.org/wiki/UTF-16#Code_points_from_U+010000_to_U+10FFFF
     */
    unsigned long Parser::decode_utf16(uint16_t i1, uint16_t i2) {
        if (!(i1 >= 0xD800 && i1 <= 0xDFFF) || !(i2 >= 0xDC00 && i2 <= 0xDFFF)) {
            //Invalid
            save_error(ERROR_SYNTAX, "Invalid surrogate pair in Unicode escape.");
            
            return 0;
        } else {
            //Valid
            i1 = i1 - 0xD800;
            i2 = i2 - 0xDC00;
            
            unsigned long U_ = (i1 << 10) | i2;
            unsigned long U = U_ + 0x10000;
            
            return U;
        }
    }

    /*
     Reads the next 4 characters as a hex integer.
     */
    uint16_t Parser::read_codepoint() {
        char buff[5];

        for (size_t i = 0; i < 4; ++i) {
            buff[i] = pop();
            
            if (buff[i] == '\0') {
                save_error(ERROR_SYNTAX, "Invalid Unicode escape in string.");

                return 0;
            }
        }

        buff[4] = '\0';

        uint16_t code_point = std::strtoul(buff, nullptr, 16);
        
        return code_point;
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
			return JSONObject(jacc::JSON_NULL());
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
