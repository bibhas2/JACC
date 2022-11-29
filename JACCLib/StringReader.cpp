#include "StringReader.h"

namespace jacc {
	StringReader::StringReader(std::string_view source) : location(0), data(source) {

	}

	StringReader::~StringReader() {
		data = {};
		location = 0;
	}

	char StringReader::peek() {
		if (location < data.size()) {
			return data.at(location);
		}
		else {
			return '\0';
		}
	}
	char StringReader::pop() {
		if (location < data.size()) {
			char result = data.at(location);

			++location;

			return result;
		}
		else {
			return '\0';
		}
	}

	void StringReader::putback() {
		if (location > 0) {
			--location;
		}
	}
}