#ifndef LOCAL_FOLDER_H
#define LOCAL_FOLDER_H

#include <filesystem>
#include <vector>
#include <string>
#include "FileInfo.h"

class LocalFolder {
public:
    // конструктор — принимает путь к папке
    LocalFolder(const std::string& path);

    void setPath(const std::string& newPath);
    void scan();
    const std::vector<FileInfo>& getFiles();
    std::vector<char> readFile(const std::string& filename);

    // можем проверить существует папка или нет
    bool exists() {
        return std::filesystem::exists(folderPath);
    }

private:
    std::filesystem::path folderPath;
    std::vector<FileInfo> filesList;
};

#endif
