#include "MemoryMappedReader.h"

namespace jacc {
	MemoryMappedReader::MemoryMappedReader(const char* file_name) : StringReader() {
#ifdef _WIN32
        file_handle = ::CreateFileA(file_name, GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (file_handle == INVALID_HANDLE_VALUE)
        {
            return;
        }

        map_handle = ::CreateFileMappingA(file_handle, NULL, PAGE_READONLY, 0, 0, NULL);

        if (map_handle == INVALID_HANDLE_VALUE)
        {
            return;
        }

        const char* buff = (const char*) ::MapViewOfFile(map_handle, FILE_MAP_READ, 0, 0, 0);

        if (buff == NULL) {
            return;
        }

        DWORD file_size = ::GetFileSize(file_handle, NULL);

        data = std::string_view(buff, file_size);
#endif
	}

    MemoryMappedReader::~MemoryMappedReader() {
#ifdef _WIN32
        if (map_handle == INVALID_HANDLE_VALUE) {
            ::CloseHandle(map_handle);

            map_handle = INVALID_HANDLE_VALUE;
        }
        if (file_handle == INVALID_HANDLE_VALUE) {
            ::CloseHandle(file_handle);

            file_handle = INVALID_HANDLE_VALUE;
        }
#endif
    }
}