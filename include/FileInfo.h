#ifndef FILE_INFO_H
#define FILE_INFO_H

#include <string>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

struct FileInfo {
    std::string name;
    std::string extension;
    uintmax_t size; 
    fs::path fullPath;
    fs::file_time_type lastWriteTime;
    bool isDirectory;
    bool exists;
  //  std::string cloudId; 
    
    FileInfo() : size(0), isDirectory(false), exists(false) {}
    explicit FileInfo(const std::string& pathStr);

    std::string getFormattedSize() const;
};

class FileAnalyzer {
public:
    static FileInfo getDetails(const std::string& pathStr);
};

#endif
