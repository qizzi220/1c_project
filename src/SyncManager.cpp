#include "SyncManager.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>

void SyncManager::initialize(const std::string& configPath) {
    std::cout << "инициализация. Конфиг: " << configPath << std::endl;
    
    // коннектимся
    if (!m_cloudApi->connect()) {
        throw std::runtime_error("ошибка подключения к облаку");
    }
}

void SyncManager::startSync() {
    std::cout << "запуск цикла синхронизации..." << std::endl;

    // сканируем директорию
    m_localFolder.scan();
    auto localFiles = m_localFolder.getFiles();

    // получаем список файлов из облака
    auto cloudFiles = m_cloudApi->getCloudFiles();

    // сравниваем и синхронизируем
    for (const auto& local : localFiles) {
        // ищем соответствие по имени файла (сравниваем имя из FileInfo)
        auto it = std::find_if(cloudFiles.begin(), cloudFiles.end(),
            [&local](const FileInfo& cloud) { 
                return local.name == cloud.name; 
            });

        if (it == cloudFiles.end()) {
            // файла нет в облаке — загружаем
            upload(local);
        } else {
            // файл есть везде — сравниваем время последней записи
            compareAndResolve(local, *it);
            // удаляем из списка, чтобы в конце остались только уникальные облачные файлы
            cloudFiles.erase(it);
        }
    }

    // оставшиеся в cloudFiles файлы есть только в облаке
    for (const auto& cloud : cloudFiles) {
        download(cloud);
    }
}

void SyncManager::compareAndResolve(const FileInfo& local, const FileInfo& cloud) {
    if (local.lastWriteTime > cloud.lastWriteTime) {
        upload(local);
    } 
    else if (cloud.lastWriteTime > local.lastWriteTime) {
        download(cloud);
    }
}

void SyncManager::upload(const FileInfo& file) {
    if (m_cloudApi->uploadFile(file)) {
        std::cout << "[SyncManager] Uploaded: " << file.name << std::endl;
    }
}

void SyncManager::download(const FileInfo& file) {
    m_cloudApi->downloadFile(file.name);
}