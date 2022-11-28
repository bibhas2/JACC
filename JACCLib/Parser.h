#pragma once

#include <string>
#include <map>
#include <vector>
#include <string_view>

namespace jacc {
	enum ErrorCode {
		ERROR_NONE,
		ERROR_INVALID_TYPE,
		ERROR_SYNTAX
	};

	enum JSONType {
		JSON_UNDEFINED,
		JSON_STRING,
		JSON_NUMBER,
		JSON_OBJECT,
		JSON_ARRAY,
		JSON_BOOLEAN,
		JSON_NULL
	};
	
	struct JSONObject {
		JSONType type;

		union {
			std::string_view str;
			double number;
			std::map<std::string_view, JSONObject>  object;
			std::vector<JSONObject> array;
			bool booleanValue;
			bool isNull;
		} value;
	};
	
	class Parser
	{
	public:
		std::string_view data;
		size_t location = 0;

		char peek();
		char pop();
		void putback();
		void eat_space();

		void parse(std::string_view source);
	};
}


