#pragma once

#include "StringReader.h"

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

namespace jacc {
	struct MemoryMappedReader :
		public StringReader
	{
#ifdef _WIN32
		HANDLE file_handle = INVALID_HANDLE_VALUE;
		HANDLE map_handle = INVALID_HANDLE_VALUE;
#endif
#ifndef _WIN32
        int file_handle = -1;
        size_t file_size = 0;
#endif
		MemoryMappedReader(const char* file_name);
		virtual ~MemoryMappedReader();
	};
}


