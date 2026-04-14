#include "../include/LocalFolder.h"
#include <fstream>
#include <iostream>
#include <system_error>

namespace fs = std::filesystem;

// Конструктор
LocalFolder::LocalFolder(const fs::path& path) : folderPath(path) {
    scan();
}

// Изменение рабочей папки с автоматическим пересканированием
void LocalFolder::setPath(const fs::path& newPath) {
    folderPath = newPath;
    scan();
}

// Глубокое сканирование папки
void LocalFolder::scan() {
    filesList.clear();
    std::error_code ec;

    if (!fs::exists(folderPath, ec)) {
        std::cerr << "[LocalFolder] Ошибка: Путь не существует: " << folderPath << std::endl;
        return;
    }

    if (!fs::is_directory(folderPath, ec)) {
        std::cerr << "[LocalFolder] Ошибка: Путь не является папкой: " << folderPath << std::endl;
        return;
    }

    // Рекурсивный обход всех вложенных файлов
    auto iterator = fs::recursive_directory_iterator(
        folderPath, 
        fs::directory_options::skip_permission_denied, 
        ec
    );

    for (const auto& entry : iterator) {
        if (ec) {
            std::cerr << "[LocalFolder] Ошибка доступа при сканировании" << std::endl;
            break;
        }

        if (entry.is_regular_file(ec)) {
            // Используем FileAnalyzer для получения метаданных (размер, время, путь)
            filesList.push_back(FileAnalyzer::getDetails(entry.path()));
        }
    }
    
    std::cout << "[LocalFolder] Сканирование завершено. Найдено файлов: " << filesList.size() << std::endl;
}

// геттер списка файлов
const std::vector<FileInfo>& LocalFolder::getFiles() const {
    return filesList;
}

// Оптимизированное чтение файла в память
std::vector<char> LocalFolder::readFile(const std::string& filename) {
    std::vector<char> data;
    
    fs::path fullPath = folderPath / filename;

    if (!fs::exists(fullPath)) {
        std::cerr << "[LocalFolder] Файл не найден: " << fullPath << std::endl;
        return data;
    }

    // ios::ate сразу прыгает в конец для определения размера
    std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
    
    if (!file.is_open()) {
        std::cerr << "[LocalFolder] Не удалось открыть файл: " << fullPath << std::endl;
        return data;
    }

    std::streamsize size = file.tellg(); 
    file.seekg(0, std::ios::beg);        

    data.resize(static_cast<size_t>(size));
    
    if (file.read(data.data(), size)) {
        // Успешно прочитано
    } else {
        data.clear();
    }

    return data;
}