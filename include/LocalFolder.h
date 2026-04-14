#ifndef LOCAL_FOLDER_H
#define LOCAL_FOLDER_H

#include <vector>
#include <filesystem>
#include "FileInfo.h"

class LocalFolder {
public:
    LocalFolder(const std::filesystem::path& path);
    void setPath(const std::filesystem::path& newPath);
    void scan();
    const std::vector<FileInfo>& getFiles() const;
    std::filesystem::path getPath() const { return folderPath; }
    std::vector<char> readFile(const std::string& filename);

private:
    std::filesystem::path folderPath;
    std::vector<FileInfo> filesList;
};

#endif