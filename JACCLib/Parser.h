#pragma once

#include <string>
#include <variant>
#include <map>
#include <vector>
#include <string_view>

namespace jacc {
	enum ErrorCode : char {
		ERROR_NONE,
		ERROR_INVALID_TYPE,
		ERROR_SYNTAX
	};

    struct JSON_UNDEFINED{};
    struct JSON_NULL{};

	struct JSONObject {
        std::variant<JSON_UNDEFINED, JSON_NULL, std::string, double, std::map<std::string, JSONObject>, std::vector<JSONObject>, bool> value;
		
		JSONObject();
        JSONObject(JSON_NULL n);
		JSONObject(std::string& s);
		JSONObject(const char* s);
		JSONObject(std::map<std::string, JSONObject>& o);
		JSONObject(std::vector<JSONObject>& a);
		JSONObject(double n);
		JSONObject(bool b);
		JSONObject(JSONObject&& other) noexcept;

		//Disable any copying. Deep copying can be very
		//expensive for a nested class like JSONObject.
		JSONObject(const JSONObject& other) = delete;
		JSONObject& operator=(const JSONObject& other) = delete;

		JSONObject& operator=(JSONObject&& other) noexcept;
        
        JSONObject& operator[](const std::string& index);
        JSONObject& operator[](const char* index);
        JSONObject& operator[](std::size_t index);
        JSONObject& operator[](int index);
        
        //Type conversion operators
        operator double();
        operator std::string&();

        bool isUndefined();
        bool isNull();
        bool isString();
        bool isNumber();
        bool isObject();
        bool isArray();
        bool isBoolean();
        
        std::string& string();
        double number();
        std::map<std::string, JSONObject>& object();
        std::vector<JSONObject>& array();
        bool boolean();
	};
	
	struct Reader
	{
		virtual char peek() = 0;
		virtual char pop() = 0;
		virtual void putback() = 0;
		virtual ~Reader() {};
	};

	class Parser
	{
	public:
		Reader& reader;
		ErrorCode error_code = ERROR_NONE;
		const char* error_message = nullptr;
		std::string value_token;

		Parser(Reader& r);
		char peek();
		char pop();
		void putback();
		void eat_space();
		void read_value_token();
		void read_quoted_string(std::string& s);
		void save_error(ErrorCode code, const char* msg);
        uint16_t read_codepoint();
        unsigned long decode_utf16(uint16_t i1, uint16_t i2);
        JSONObject parse();
		JSONObject parse_value();
		JSONObject parse_array();
		JSONObject parse_string();
		JSONObject parse_object();
		JSONObject parse_number();
		JSONObject parse_bool();
		JSONObject parse_null();
	};
}


