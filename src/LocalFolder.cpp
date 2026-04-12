#include "LocalFolder.h"
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;
using namespace std;

// конструктор
LocalFolder::LocalFolder(const string& path) {
    folderPath = path;
    scan();
}

// меняем папку
void LocalFolder::setPath(const string& newPath) {
    folderPath = newPath;
    scan();
}

// сканируем
void LocalFolder::scan() {
    filesList.clear();
    cout << "Сканирование папкии: " << folderPath << endl;
    if (!fs::exists(folderPath)) {
        cout << "Папки не существует" << endl;
        return;
    }
    
    // обходим все папки и файлы
    for (const auto& entry : fs::recursive_directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            // обычный файл — добавляем в список
            FileInfo f(entry.path().string());
            filesList.push_back(f);
        }
    }
    cout << "Найдено: " << filesList.size() << " файлов" << endl;
}

// получаем список файлов
const vector<FileInfo>& LocalFolder::getFiles() {
    return filesList;
}

// читаем файлик
vector<char> LocalFolder::readFile(const string& filename) {
    vector<char> data;
    fs::path fullPath = folderPath;
    fullPath /= filename;
    if (!fs::exists(fullPath)) {
        cout << "Нет файла: " << filename << endl;
        return data;
    }
    
    ifstream file(fullPath, ios::binary);
    
    if (!file.is_open()) {
        cout << "Не удалось открыть: " << filename << endl;
        return data;
    }
    
    // смотрим размер файла
    file.seekg(0, ios::end);
    int len = file.tellg();
    file.seekg(0, ios::beg);
    
    // читаем содержимое
    data.resize(len);
    file.read(data.data(), len);
    file.close();
    
    cout << "Файл прочитан: " << filename << " (" << len << " байт)" << endl;
    
    return data;
}
