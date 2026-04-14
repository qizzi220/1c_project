#include "../include/SyncManager.h"
#include <iostream>
#include <unordered_map>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

SyncManager::SyncManager(std::shared_ptr<CloudApi> api, const std::filesystem::path& folderPath)
    : m_cloudApi(std::move(api)), m_localFolder(folderPath) 
{
}
// Инициализация менеджера
void SyncManager::initialize(const std::string& configPath) {
    std::cout << "[SyncManager] Инициализация. Конфиг: " << configPath << std::endl;
    
    if (!m_cloudApi) {
        throw std::runtime_error("Cloud API не инициализирован");
    }

    // Проверяем соединение с облаком (Google Drive / Yandex)
    if (!m_cloudApi->connect()) {
        throw std::runtime_error("Не удалось подключиться к облаку. Проверьте токен.");
    }

    // Проверяем, существует ли локальная папка
    if (!fs::exists(m_localFolder.getPath())) {
        std::cout << "[SyncManager] Локальная папка не найдена. Создаю: " << m_localFolder.getPath() << std::endl;
        fs::create_directories(m_localFolder.getPath());
    }
}

// Основной цикл синхронизации
void SyncManager::startSync() {
    std::cout << "[SyncManager] Начало процесса синхронизации..." << std::endl;

    // 1. Сканируем локальные файлы
    m_localFolder.scan();
    auto localFiles = m_localFolder.getFiles();

    // 2. Получаем список файлов из облака
    auto cloudFiles = m_cloudApi->getCloudFiles();

    // 3. Оптимизация: создаем карту облачных файлов для быстрого поиска по имени O(1)
    std::unordered_map<std::string, FileInfo> cloudFilesMap;
    for (const auto& cloudFile : cloudFiles) {
        cloudFilesMap[cloudFile.name] = cloudFile;
    }

    // 4. Сравниваем локальные файлы с облачными
    for (const auto& local : localFiles) {
        auto it = cloudFilesMap.find(local.name);

        if (it == cloudFilesMap.end()) {
            // Файла нет в облаке — загружаем его туда
            std::cout << "[SyncManager] Новый локальный файл: " << local.name << std::endl;
            upload(local);
        } else {
            // Файл есть и там, и там — решаем конфликт версий
            compareAndResolve(local, it->second);
            // Удаляем из карты, чтобы в конце остались только те файлы, которых нет локально
            cloudFilesMap.erase(it);
        }
    }

    // 5. Все файлы, оставшиеся в карте — есть в облаке, но их нет на диске
    for (const auto& [name, cloudInfo] : cloudFilesMap) {
        std::cout << "[SyncManager] Новый файл в облаке: " << name << std::endl;
        download(cloudInfo);
    }

    std::cout << "[SyncManager] Синхронизация успешно завершена." << std::endl;
}

// Логика разрешения конфликтов (Last Write Wins)
void SyncManager::compareAndResolve(const FileInfo& local, const FileInfo& cloud) {
    // Сравниваем время последнего изменения (lastWriteTime в секундах)
    if (local.lastWriteTime > cloud.lastWriteTime) {
        // Локальный файл изменен позже — обновляем облако
        std::cout << "[SyncManager] Обновление облака: " << local.name << " (локальный новее)" << std::endl;
        upload(local);
    } 
    else if (cloud.lastWriteTime > local.lastWriteTime) {
        // Облачный файл изменен позже — обновляем локальный диск
        std::cout << "[SyncManager] Обновление локального файла: " << cloud.name << " (облачный новее)" << std::endl;
        download(cloud);
    }
    // Если время совпадает секунда в секунду — файлы идентичны, ничего не делаем
}

// Обертка над методом загрузки API
void SyncManager::upload(const FileInfo& file) {
    if (m_cloudApi->uploadFile(file)) {
        std::cout << " [↑] Загружено: " << file.name << std::endl;
    } else {
        std::cerr << " [!] Ошибка загрузки: " << file.name << std::endl;
    }
}

// Обертка над методом скачивания API
void SyncManager::download(const FileInfo& file) {
    // Собираем кроссплатформенный путь: путь_к_папке / имя_файла
    fs::path destination = fs::path(m_localFolder.getPath()) / file.name;

    if (m_cloudApi->downloadFile(file.name, destination)) {
        std::cout << " [↓] Скачано: " << file.name << std::endl;
    } else {
        std::cerr << " [!] Ошибка скачивания: " << file.name << std::endl;
    }
}