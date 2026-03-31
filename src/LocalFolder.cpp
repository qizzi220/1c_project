#include "LocalFolder.h"
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

//конструктор
LocalFolder::LocalFolder(const std::string& path) {
    folderPath = fs::path(path);
    scan(); //сразу сканируем папку
}

//сменить папку
void LocalFolder::setPath(const std::string& newPath) {
    folderPath = fs::path(newPath);
    scan();
}

//сканирование папки
void LocalFolder::scan() {
    filesList.clear(); //очищаем старый список
    std::cout << "Сканирование папки: " << folderPath << std::endl;
        
    //проверяем, существует ли папка
    if (!fs::exists(folderPath)) {
        std::cout << "Папка не существует" << std::endl;
        return;
    }
    
    //проходимся по всем файлам в папке
    for (const auto& entry : fs::recursive_directory_iterator(folderPath)) {
        if (entry.is_regular_file()) { //если это файл,то
            //создаём объект FileInfo для каждого файла
            FileInfo fileInfo(entry.path().string());
            filesList.push_back(fileInfo);
        }
    }
    
    std::cout << "Найдено файлов: " << filesList.size() << std::endl;
}

//получаем список файлов
const std::vector<FileInfo>& LocalFolder::getFiles() const {
    return filesList;
}

//прочитать содержимое файла
std::vector<char> LocalFolder::readFile(const std::string& filename) const {
    std::vector<char> buffer;
    
    //собираем полный путь к файлу
    fs::path fullPath = folderPath / filename;
    
    //проверяем, существует файл или нет
    if (!fs::exists(fullPath)) {
        std::cout << "Файл не найден: " << filename << std::endl;
        return buffer; //возвращаем пустой вектор
    }
    
    //открываем файл для чтения в бинарном режиме
    std::ifstream file(fullPath, std::ios::binary);
    
    if (file.is_open()) {
        //определяем размер файла
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        //выделяем память и читаем файл
        buffer.resize(size);
        file.read(buffer.data(), size);
        
        std::cout << "Файл прочитан: " << filename << " (" << size << " байт)" << std::endl;
    } 
    else {
        std::cout << "Не удалось открыть файл: " << filename << std::endl;
    }
    
    return buffer;
}