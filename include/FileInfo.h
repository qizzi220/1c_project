#ifndef FILE_INFO_H
#define FILE_INFO_H

#include <string>
#include <filesystem>
#include <ctime>

namespace fs = std::filesystem;

struct FileInfo {
    std::string name;
    std::string extension;
    fs::path fullPath;
    bool exists = false;
    bool isDirectory = false;
    uintmax_t size = 0;
    std::time_t lastWriteTime = 0;
    std::string parentFolder;

    FileInfo() = default;
    FileInfo(const std::string& pathStr);
    std::string getFormattedSize() const;
};

class FileAnalyzer {
public:
    static FileInfo getDetails(const fs::path& p);
};

#endif