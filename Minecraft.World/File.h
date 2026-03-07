#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class FileFilter;

class File
{
public:
    static const char pathSeparator;

    explicit File(const std::string& pathname);
    File(const File& parent, const std::string& child);
    File(const std::string& parent, const std::string& child);

    bool exists() const;
    bool isFile() const;
    bool isDirectory() const;

    bool _delete();
    bool mkdir() const;
    bool mkdirs() const;
    bool renameTo(const File& dest);

    int64_t length() const;
    int64_t     lastModified() const;

    std::string getPath() const;
    std::string getName() const;
    std::string getParentPath() const;

    using FileList = std::vector<std::unique_ptr<File>>;

    std::unique_ptr<FileList> listFiles() const;
    std::unique_ptr<FileList> listFiles(FileFilter* filter) const;

    static bool eq_test(const File& x, const File& y);
    static int  hash_fnct(const File& k);

private:
    std::string m_path;
};