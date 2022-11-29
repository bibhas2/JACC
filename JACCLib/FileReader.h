#pragma once
#include "Parser.h"
#include <fstream>

namespace jacc {
	struct FileReader :
		public Reader
	{
		std::ifstream file;

		FileReader(const char* source);

		char peek();
		char pop();
		void putback();

		virtual ~FileReader();
	};
}


