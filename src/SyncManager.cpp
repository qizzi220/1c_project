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
// инициализация
void SyncManager::initialize(const std::string& configPath) {
    std::cout << "[SyncManager] Инициализация. Конфиг: " << configPath << std::endl;
    
    if (!m_cloudApi) {
        throw std::runtime_error("Cloud API не инициализирован");
    }

    // проверка соединения с облаком
    if (!m_cloudApi->connect()) {
        throw std::runtime_error("Не удалось подключиться к облаку. Проверьте токен.");
    }

    if (!fs::exists(m_localFolder.getPath())) {
        std::cout << "[SyncManager] Локальная папка не найдена. Создаю: " << m_localFolder.getPath() << std::endl;
        fs::create_directories(m_localFolder.getPath());
    }
}

void SyncManager::startSync() {
    std::cout << "[SyncManager] Начало процесса синхронизации..." << std::endl;

    m_localFolder.scan();
    auto localFiles = m_localFolder.getFiles();

    auto cloudFiles = m_cloudApi->getCloudFiles();

    // Создание мапы облачных файлов за О(1),что оптимизирует наш код
    std::unordered_map<std::string, FileInfo> cloudFilesMap;
    for (const auto& cloudFile : cloudFiles) {
        cloudFilesMap[cloudFile.name] = cloudFile;
    }

    for (const auto& local : localFiles) {
        auto it = cloudFilesMap.find(local.name);

        if (it == cloudFilesMap.end()) {
            std::cout << "[SyncManager] Новый локальный файл: " << local.name << std::endl;
            upload(local);
        } else {
            compareAndResolve(local, it->second);
            // Удаляем из мапы, чтобы в конце остались только те файлы, которых нет локально
            cloudFilesMap.erase(it);
        }
    }

    // все файлы, оставшиеся в мапе — есть в облаке, но их нет на диске
    for (const auto& [name, cloudInfo] : cloudFilesMap) {
        std::cout << "[SyncManager] Новый файл в облаке: " << name << std::endl;
        download(cloudInfo);
    }

    std::cout << "[SyncManager] Синхронизация успешно завершена." << std::endl;
}

// Last Write Wins
void SyncManager::compareAndResolve(const FileInfo& local, const FileInfo& cloud) {
    if (local.lastWriteTime > cloud.lastWriteTime) {
        std::cout << "[SyncManager] Обновление облака: " << local.name << " (локальный новее)" << std::endl;
        upload(local);
    } 
    else if (cloud.lastWriteTime > local.lastWriteTime) {
        std::cout << "[SyncManager] Обновление локального файла: " << cloud.name << " (облачный новее)" << std::endl;
        download(cloud);
    }
    // файлы идентичны, ничего не делаем
}

// обертка над методом загрузки API
void SyncManager::upload(const FileInfo& file) {
    if (m_cloudApi->uploadFile(file)) {
        std::cout << " [↑] Загружено: " << file.name << std::endl;
    } else {
        std::cerr << " [!] Ошибка загрузки: " << file.name << std::endl;
    }
}

// О/обертка над методом скачивания API
void SyncManager::download(const FileInfo& file) {
    // собираем кроссплатформенный путь: путь_к_папке / имя_файла
    fs::path destination = fs::path(m_localFolder.getPath()) / file.name;

    if (m_cloudApi->downloadFile(file.name, destination)) {
        std::cout << " [↓] Скачано: " << file.name << std::endl;
    } else {
        std::cerr << " [!] Ошибка скачивания: " << file.name << std::endl;
    }
}