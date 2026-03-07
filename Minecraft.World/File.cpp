#include "File.h"
#include "FileFilter.h"

#include <cassert>
#include <cstdint>
#include <memory>
#include <stdexcept>

#if defined(_WIN32)
    #define PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#else
    #define PLATFORM_POSIX
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <dirent.h>
    #include <unistd.h>
    #include <cerrno>
    #include <cstring>
#endif

#if defined(PLATFORM_WINDOWS)
const char File::pathSeparator = '\\';
#else
const char File::pathSeparator = '/';
#endif

namespace {

std::vector<std::string> splitPath(const std::string& path, char sep)
{
    std::vector<std::string> parts;
    std::string token;
    for (char c : path) {
        if (c == sep) {
            parts.push_back(token);
            token.clear();
        } else {
            token += c;
        }
    }
    parts.push_back(token);
    return parts;
}

}

File::File(const std::string& pathname)
    : m_path(pathname)
{}

File::File(const File& parent, const std::string& child)
    : m_path(parent.getPath() + pathSeparator + child)
{}

File::File(const std::string& parent, const std::string& child)
    : m_path(parent + pathSeparator + child)
{}

bool File::exists() const
{
#if defined(PLATFORM_WINDOWS)
    DWORD attr = GetFileAttributesA(m_path.c_str());
    return attr != INVALID_FILE_ATTRIBUTES;
#else
    struct stat st;
    return ::stat(m_path.c_str(), &st) == 0;
#endif
}

bool File::isFile() const
{
#if defined(PLATFORM_WINDOWS)
    DWORD attr = GetFileAttributesA(m_path.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) return false;
    return !(attr & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat st;
    if (::stat(m_path.c_str(), &st) != 0) return false;
    return S_ISREG(st.st_mode);
#endif
}

bool File::isDirectory() const
{
#if defined(PLATFORM_WINDOWS)
    DWORD attr = GetFileAttributesA(m_path.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) return false;
    return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
    struct stat st;
    if (::stat(m_path.c_str(), &st) != 0) return false;
    return S_ISDIR(st.st_mode);
#endif
}

bool File::_delete()
{
#if defined(PLATFORM_WINDOWS)
    if (isDirectory())
        return RemoveDirectoryA(m_path.c_str()) != 0;
    return DeleteFileA(m_path.c_str()) != 0;
#else
    return ::remove(m_path.c_str()) == 0;
#endif
}

bool File::mkdir() const
{
#if defined(PLATFORM_WINDOWS)
    return CreateDirectoryA(m_path.c_str(), nullptr) != 0;
#else
    return ::mkdir(m_path.c_str(), 0755) == 0;
#endif
}

bool File::mkdirs() const
{
    if (exists()) return isDirectory();

    std::vector<std::string> parts = splitPath(m_path, pathSeparator);
    std::string current;

    for (size_t i = 0; i < parts.size(); ++i) {
        if (i == 0) {
            current = parts[i];   // may be "" for a POSIX absolute path
        } else {
            current += pathSeparator;
            current += parts[i];
        }
        if (current.empty() || current == ".") continue;

#if defined(PLATFORM_WINDOWS)
        // Skip drive roots like "C:" 
        if (current.size() == 2 && current[1] == ':') continue;
        DWORD attr = GetFileAttributesA(current.c_str());
        if (attr == INVALID_FILE_ATTRIBUTES) {
            if (!CreateDirectoryA(current.c_str(), nullptr)) return false;
        } else if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
            return false; // A file is blocking the path
        }
#else
        struct stat st;
        if (::stat(current.c_str(), &st) != 0) {
            if (::mkdir(current.c_str(), 0755) != 0 && errno != EEXIST)
                return false;
        } else if (!S_ISDIR(st.st_mode)) {
            return false; // A file is blocking the path
        }
#endif
    }

    assert(exists());
    return true;
}

bool File::renameTo(const File& dest)
{
#if defined(PLATFORM_WINDOWS)
    return MoveFileA(m_path.c_str(), dest.getPath().c_str()) != 0;
#else
    return ::rename(m_path.c_str(), dest.getPath().c_str()) == 0;
#endif
}

int64_t File::length() const
{
#if defined(PLATFORM_WINDOWS)
    WIN32_FILE_ATTRIBUTE_DATA info;
    if (!GetFileAttributesExA(m_path.c_str(), GetFileExInfoStandard, &info))
        return 0;
    if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        return 0;
    LARGE_INTEGER li;
    li.HighPart = static_cast<LONG>(info.nFileSizeHigh);
    li.LowPart  = info.nFileSizeLow;
    return static_cast<int64_t>(li.QuadPart);
#else
    struct stat st;
    if (::stat(m_path.c_str(), &st) != 0) return 0;
    if (S_ISDIR(st.st_mode)) return 0;
    return static_cast<int64_t>(st.st_size);
#endif
}

int64_t File::lastModified() const
{
#if defined(PLATFORM_WINDOWS)
    WIN32_FILE_ATTRIBUTE_DATA info;
    if (!GetFileAttributesExA(m_path.c_str(), GetFileExInfoStandard, &info))
        return 0;
    if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        return 0;

    LARGE_INTEGER li;
    li.HighPart = static_cast<LONG>(info.ftLastWriteTime.dwHighDateTime);
    li.LowPart  = info.ftLastWriteTime.dwLowDateTime;

    const int64_t EPOCH_DIFF = 116444736000000000LL;
    return (li.QuadPart - EPOCH_DIFF) / 10000;
#else
    struct stat st;
    if (::stat(m_path.c_str(), &st) != 0) return 0;
    if (S_ISDIR(st.st_mode)) return 0;
    return static_cast<int64_t>(st.st_mtime) * 1000;
#endif
}

std::string File::getPath() const
{
    return m_path;
}

std::string File::getName() const
{
    size_t sep = m_path.find_last_of(pathSeparator);
    if (sep == std::string::npos) return m_path;
    return m_path.substr(sep + 1);
}

std::string File::getParentPath() const
{
    size_t sep = m_path.find_last_of(pathSeparator);
    if (sep == std::string::npos) return "";
    if (sep == 0) return std::string(1, pathSeparator);
    return m_path.substr(0, sep);
}

std::unique_ptr<File::FileList> File::listFiles() const
{
    auto output = std::unique_ptr<FileList>(new FileList());
    if (!isDirectory()) return output;

#if defined(PLATFORM_WINDOWS)
    std::string pattern = m_path + pathSeparator + "*";
    WIN32_FIND_DATAA wfd;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &wfd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::string name = wfd.cFileName;
            if (name == "." || name == "..") continue;
            output->push_back(std::unique_ptr<File>(new File(*this, name)));
        } while (FindNextFileA(hFind, &wfd));
        FindClose(hFind);
    }
#else
    DIR* dir = ::opendir(m_path.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = ::readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name == "." || name == "..") continue;
            output->push_back(std::unique_ptr<File>(new File(*this, name)));
        }
        ::closedir(dir);
    }
#endif

    return output;
}

std::unique_ptr<File::FileList> File::listFiles(FileFilter* filter) const
{
    if (!isDirectory()) return nullptr;

    auto output = std::unique_ptr<FileList>(new FileList());

#if defined(PLATFORM_WINDOWS)
    std::string pattern = m_path + pathSeparator + "*";
    WIN32_FIND_DATAA wfd;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &wfd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::string name = wfd.cFileName;
            if (name == "." || name == "..") continue;
            auto candidate = std::unique_ptr<File>(new File(*this, name));
            if (filter && filter->accept(*candidate))
                output->push_back(std::move(candidate));
            // unique_ptr destructs candidate automatically if not accepted
        } while (FindNextFileA(hFind, &wfd));
        FindClose(hFind);
    }
#else
    DIR* dir = ::opendir(m_path.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = ::readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name == "." || name == "..") continue;
            auto candidate = std::unique_ptr<File>(new File(*this, name));
            if (filter && filter->accept(*candidate))
                output->push_back(std::move(candidate));
            // unique_ptr destructs candidate automatically if not accepted
        }
        ::closedir(dir);
    }
#endif

    return output;
}

bool File::eq_test(const File& x, const File& y)
{
#if defined(PLATFORM_WINDOWS)
    // Windows paths are case-insensitive
    if (x.m_path.size() != y.m_path.size()) return false;
    for (size_t i = 0; i < x.m_path.size(); ++i) {
        if (::tolower((unsigned char)x.m_path[i]) !=
            ::tolower((unsigned char)y.m_path[i]))
            return false;
    }
    return true;
#else
    return x.m_path == y.m_path;
#endif
}

int File::hash_fnct(const File& k)
{
    // djb2 — simple, well-distributed for path strings
    unsigned long hash = 5381;
    for (unsigned char c : k.m_path)
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    return static_cast<int>(hash);
}