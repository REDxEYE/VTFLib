#pragma once

#include <cstdint>

#ifdef _WIN32

#include <windows.h>

namespace winfind_detail {
    inline const ::HANDLE invalid_handle_value = INVALID_HANDLE_VALUE;
    constexpr std::uint32_t file_attribute_directory = FILE_ATTRIBUTE_DIRECTORY;
}

// Required so winfind::INVALID_HANDLE_VALUE and
// winfind::FILE_ATTRIBUTE_DIRECTORY don't get macro-expanded.
// #undef INVALID_HANDLE_VALUE
// #undef FILE_ATTRIBUTE_DIRECTORY

#else

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

#endif


namespace winfind {
#ifdef _WIN32

    using HANDLE = ::HANDLE;
    using WIN32_FIND_DATA = ::WIN32_FIND_DATAA;


    inline HANDLE FindFirstFile(
        const char *searchPattern,
        WIN32_FIND_DATA *findData) {
        return ::FindFirstFileA(searchPattern, findData);
    }

    inline bool FindNextFile(
        HANDLE handle,
        WIN32_FIND_DATA *findData) {
        return ::FindNextFileA(handle, findData) != FALSE;
    }

    inline bool FindClose(HANDLE handle) {
        return ::FindClose(handle) != FALSE;
    }


#else


    using DWORD = std::uint32_t;

    struct WIN32_FIND_DATA {
        DWORD dwFileAttributes{};
        char cFileName[260]{};
    };


    struct FindHandle {
        std::vector<std::filesystem::directory_entry> entries;
        std::size_t index = 0;
    };


    using HANDLE = FindHandle *;

    inline const HANDLE _INTERNAL_INVALID_HANDLE_VALUE = reinterpret_cast<HANDLE>(static_cast<std::intptr_t>(-1));
    constexpr DWORD _INTERNAL_FILE_ATTRIBUTE_DIRECTORY = 0x00000010;

    inline std::string NormalizePath(std::string path) {
        std::replace(path.begin(), path.end(), '\\', '/');
        return path;
    }


    inline bool WildcardMatch(
        const char *pattern,
        const char *value) {
        while (*pattern) {
            if (*pattern == '*') {
                ++pattern;

                if (!*pattern)
                    return true;

                while (*value) {
                    if (WildcardMatch(pattern, value))
                        return true;

                    ++value;
                }

                return false;
            }

            if (!*value)
                return false;

            if (*pattern != '?') {
                const auto patternChar =
                        static_cast<unsigned char>(*pattern);

                const auto valueChar =
                        static_cast<unsigned char>(*value);

                if (std::tolower(patternChar) != std::tolower(valueChar))
                    return false;
            }

            ++pattern;
            ++value;
        }

        return *value == '\0';
    }


    inline void FillFindData(
        const std::filesystem::directory_entry &entry,
        WIN32_FIND_DATA *findData) {
        std::error_code ec;

        findData->dwFileAttributes =
                entry.is_directory(ec)
                    ? _INTERNAL_FILE_ATTRIBUTE_DIRECTORY
                    : 0;

        const std::string name =
                entry.path().filename().string();

        std::strncpy(
            findData->cFileName,
            name.c_str(),
            sizeof(findData->cFileName) - 1
        );

        findData->cFileName[
            sizeof(findData->cFileName) - 1
        ] = '\0';
    }


    inline HANDLE FindFirstFile(
        const char *searchPattern,
        WIN32_FIND_DATA *findData) {
        namespace fs = std::filesystem;

        const fs::path searchPath(
            NormalizePath(searchPattern)
        );

        fs::path directory =
                searchPath.parent_path();

        const std::string wildcard =
                searchPath.filename().string();

        if (directory.empty())
            directory = ".";

        std::error_code ec;

        auto *handle = new FindHandle();

        fs::directory_iterator iterator(directory, ec);

        if (ec) {
            delete handle;
            return _INTERNAL_INVALID_HANDLE_VALUE;
        }

        for (const auto &entry: iterator) {
            const std::string name =
                    entry.path().filename().string();

            if (WildcardMatch(
                wildcard.c_str(),
                name.c_str())) {
                handle->entries.push_back(entry);
            }
        }

        if (handle->entries.empty()) {
            delete handle;
            return _INTERNAL_INVALID_HANDLE_VALUE;
        }

        FillFindData(
            handle->entries[0],
            findData
        );

        return handle;
    }


    inline bool FindNextFile(
        HANDLE handle,
        WIN32_FIND_DATA *findData) {
        if (!handle ||
            handle == _INTERNAL_INVALID_HANDLE_VALUE) {
            return false;
        }

        ++handle->index;

        if (handle->index >= handle->entries.size())
            return false;

        FillFindData(
            handle->entries[handle->index],
            findData
        );

        return true;
    }


    inline bool FindClose(HANDLE handle) {
        if (!handle ||
            handle == _INTERNAL_INVALID_HANDLE_VALUE) {
            return false;
        }

        delete handle;
        return true;
    }

#endif
}


#ifndef _WIN32
inline const winfind::HANDLE INVALID_HANDLE_VALUE = reinterpret_cast<winfind::HANDLE>(static_cast<std::intptr_t>(-1));
constexpr winfind::DWORD FILE_ATTRIBUTE_DIRECTORY = 0x00000010;

#endif
