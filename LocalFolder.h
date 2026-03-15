#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include "FileInfo.h"

namespace fs = std::filesystem;

class LocalFolder {
private:
    fs::path folderPath;
    std::vector<FileInfo> filesList;

public:
    //конструктор - принимает путь к папке
    explicit LocalFolder(const std::string& path);

    //сменить папку для синхронизации
    void setPath(const std::string& newPath);

    //просканировать папку и обновить filesList
    void scan();

    //получить текущий список файлов
    const std::vector<FileInfo>& getFiles() const;

    //прочитать содержимое файла для загрузки в облако
    std::vector<char> readFile(const std::string& filename) const;
};