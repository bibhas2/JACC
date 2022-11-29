#include "FileReader.h"

namespace jacc {
	FileReader::FileReader(const char* source) : file(source) {

	}

	FileReader::~FileReader() {
	}

	char FileReader::peek() {
		char result = file.peek();

		if (result == EOF) {
			return '\0';
		}

		return result;
	}

	char FileReader::pop() {
		char result = file.get();

		if (result == EOF) {
			return '\0';
		}

		return result;
	}

	void FileReader::putback() {
		file.unget();
	}
}