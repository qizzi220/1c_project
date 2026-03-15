#include "CloudApi.h"
#include <iostream>
#include <thread>

CloudApi::CloudApi(const std::string& token) {
    apiToken = token;
    serverUrl = "https://api....";
    timeout = 30;
    isConnected = false;
}

bool CloudApi::connect() {
    std::cout << "Подключение к облаку..." << std::endl;
    if (apiToken.empty()) {
        std::cerr << "Отсутсвует токен" << std::endl;
        return false;
    }
    //имитация задержки подключения
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    isConnected = true;
    std::cout << "Подключено к " << serverUrl << std::endl;
    return true;
}

bool CloudApi::uploadFile(const FileInfo& file) {
    if (!isConnected) {
        std::cerr << "Нет подключения к облаку" << std::endl;
        return false;
    }
    
    std::cout << "Загрузка файла: " << file.name << std::endl;
    //имитация загрузки
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    std::cout << "Файл загружен" << std::endl;
    return true;
}

bool CloudApi::downloadFile(const std::string& fileName) {
    if (!isConnected) {
        std::cerr << "Нет подключения к облаку" << std::endl;
        return false;
    }
    
    std::cout << "Скачивание файла: " << fileName << std::endl;
    //имитация скачивания
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    std::cout << "Файл скачан: " << fileName << std::endl;
    return true;
}

std::vector<FileInfo> CloudApi::getCloudFiles() {
    std::vector<FileInfo> files;
    
    if (!isConnected) {
        std::cerr << "Нет подключения к облаку" << std::endl;
        return files;
    }

    std::cout << "Получение списка файлов..." << std::endl;
    //имитация получения списка
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    std::cout << "Файлы получены" << std::endl;
    //возвращаем пустой вектор, тестовые данные будут в юнит-тестах
    return files;
}