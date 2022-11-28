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

		std::string_view str;
		double number = 0.0;
		std::map<std::string_view, JSONObject>  object;
		std::vector<JSONObject> array;
		bool booleanValue = false;

		JSONObject();
		JSONObject(const std::string_view& s);
		JSONObject(std::map<std::string_view, JSONObject>& o);
		JSONObject(std::vector<JSONObject>& a);
		JSONObject(double n);
		JSONObject(bool b);
		JSONObject(JSONObject&& other);

		//Disable any copying. Deep copying can be very
		//expensive for a nested class like JSONObject.
		JSONObject(const JSONObject& other) = delete;
		JSONObject& operator=(const JSONObject& other) = delete;

		JSONObject& operator=(JSONObject&& other);
	};
	
	class Parser
	{
	public:
		std::string_view data;
		size_t location = 0;
		ErrorCode error_code = ERROR_NONE;
		const char* error_message = nullptr;
		JSONObject root;
		std::string value_token;

		Parser();
		char peek();
		char pop();
		void putback();
		void eat_space();
		void read_value_token();

		void save_error(ErrorCode code, const char* msg);
		void parse(std::string_view source);
		JSONObject parse_value();
		JSONObject parse_array();
		JSONObject parse_string();
		JSONObject parse_object();
		JSONObject parse_number();
		JSONObject parse_bool();
		JSONObject parse_null();
	};
}


