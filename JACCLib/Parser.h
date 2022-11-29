#pragma once

#include <string>
#include <map>
#include <vector>
#include <string_view>

namespace jacc {
	enum ErrorCode : char {
		ERROR_NONE,
		ERROR_INVALID_TYPE,
		ERROR_SYNTAX
	};

	enum JSONType : char {
		JSON_UNDEFINED,
		JSON_STRING,
		JSON_NUMBER,
		JSON_OBJECT,
		JSON_ARRAY,
		JSON_BOOLEAN,
		JSON_NULL
	};
	
	struct JSONObject {
		JSONType type = JSON_UNDEFINED;

		std::string str;
		double number = 0.0;
		std::map<std::string, JSONObject>  object;
		std::vector<JSONObject> array;
		bool booleanValue = false;

		JSONObject();
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
		JSONObject root;
		std::string value_token;

		Parser(Reader& r);
		char peek();
		char pop();
		void putback();
		void eat_space();
		void read_value_token();
		void read_quoted_string(std::string& s);
		void save_error(ErrorCode code, const char* msg);
		void parse();
		JSONObject parse_value();
		JSONObject parse_array();
		JSONObject parse_string();
		JSONObject parse_object();
		JSONObject parse_number();
		JSONObject parse_bool();
		JSONObject parse_null();
	};
}


