#pragma once
#include "Parser.h"

namespace jacc {
	struct StringReader :
		public Reader
	{
		size_t location = 0;
		std::string_view data;

		StringReader(std::string_view source);

		char peek();
		char pop();
		void putback();

		virtual ~StringReader();
	};
}

