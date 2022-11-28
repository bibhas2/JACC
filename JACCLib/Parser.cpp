#include "Parser.h"

namespace jacc {
	void Parser::parse(std::string_view source) {
		data = source;
		location = 0;

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
}