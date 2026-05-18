#include "../include/SyncManager.h"
#include <iostream>
#include <unordered_map>
#include <filesystem>
#include <stdexcept>
#include <fstream> 
#include <sys/types.h>
#include <sys/stat.h>

namespace fs = std::filesystem;

SyncManager::SyncManager(std::shared_ptr<CloudApi> api, const std::filesystem::path& folderPath)
    : m_cloudApi(std::move(api)), m_localFolder(folderPath) 
{
}

void SyncManager::initialize(const std::string& configPath) {
    m_configPath = configPath;
    std::cout << "[SyncManager] Инициализация. Конфиг: " << configPath << std::endl;
    
    if (!m_cloudApi) {
        throw std::runtime_error("Cloud API не инициализирован");
    }

    std::ifstream file(configPath);
    if (file.is_open()) {
        nlohmann::json j;
        file >> j;
        if (j.contains("sync_history")) {
            for (auto it = j["sync_history"].begin(); it != j["sync_history"].end(); ++it) {
                SyncState state;
                if (it.value().contains("local")) state.localTime = it.value()["local"];
                if (it.value().contains("cloud")) state.cloudTime = it.value()["cloud"];
                m_syncHistory[it.key()] = state;
            }
        }
    }

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
            cloudFilesMap.erase(it);
        }
    }

    for (const auto& [name, cloudInfo] : cloudFilesMap) {
        std::cout << "[SyncManager] Новый файл в облаке: " << name << std::endl;
        download(cloudInfo);
    }

    std::cout << "[SyncManager] Синхронизация успешно завершена." << std::endl;
}

void SyncManager::compareAndResolve(const FileInfo& local, const FileInfo& cloud) {
    std::time_t lastKnownLocal = 0;
    std::time_t lastKnownCloud = 0;
    
    if (m_syncHistory.count(local.name)) {
        lastKnownLocal = m_syncHistory[local.name].localTime;
        lastKnownCloud = m_syncHistory[local.name].cloudTime;
    }

    if (lastKnownLocal == 0 || lastKnownCloud == 0) {
        std::cout << "[SyncManager] Файл не найден в истории. Скачиваю: " << cloud.name << std::endl;
        download(cloud);
        return;
    }

    // Считаем дельты для диска и для облака отдельно
    long long localDelta = static_cast<long long>(local.lastWriteTime) - static_cast<long long>(lastKnownLocal);
    long long cloudDelta = static_cast<long long>(cloud.lastWriteTime) - static_cast<long long>(lastKnownCloud);

    bool localChanged = (localDelta > 2);
    bool cloudChanged = (cloudDelta > 2);

    if (cloudChanged && !localChanged) {
        std::cout << "[SyncManager] Файл обновился в облаке: " << cloud.name << std::endl;
        download(cloud);
    } 
    else if (localChanged && !cloudChanged) {
        std::cout << "[SyncManager] Локальный файл изменился: " << local.name << std::endl;
        upload(local);
    } 
    else if (localChanged && cloudChanged) {
        std::cout << "[CONFLICT] Файл изменен и тут, и там: " << local.name << ". Приоритет облаку." << std::endl;
        download(cloud);
    }
}

void SyncManager::saveConfig(const std::string& configPath) {
    nlohmann::json j;
    
    j["client_id"] = m_cloudApi->getClientId();
    j["client_secret"] = m_cloudApi->getClientSecret();
    j["refresh_token"] = m_cloudApi->getRefreshToken();

    for (const auto& [name, state] : m_syncHistory) {
        j["sync_history"][name]["local"] = state.localTime;
        j["sync_history"][name]["cloud"] = state.cloudTime;
    }

    std::ofstream file(configPath);
    if (file.is_open()) {
        file << j.dump(4);
        std::cout << "[SyncManager] Конфигурация и история сохранены в " << configPath << std::endl;
    }
}

void SyncManager::upload(const FileInfo& file) {
    if (m_cloudApi->uploadFile(file)) {
        std::cout << " [↑] Загружено: " << file.name << std::endl;
        
        // Запоминаем время диска. Облачное обновится при следующем сканировании
        m_syncHistory[file.name].localTime = file.lastWriteTime;
        m_syncHistory[file.name].cloudTime = file.lastWriteTime; 
    } else {
        std::cerr << " [!] Ошибка загрузки: " << file.name << std::endl;
    }
}

void SyncManager::download(const FileInfo& file) {
    fs::path destination = fs::path(m_localFolder.getPath()) / file.name;

    if (m_cloudApi->downloadFile(file.name, destination)) {
        std::cout << " [↓] Скачано: " << file.name << std::endl;
        
        // Читаем, какой таймстамп выставила ОС файлу после скачивания
        FileInfo downloadedFileInfo = FileAnalyzer::getDetails(destination);
        
        m_syncHistory[file.name].localTime = downloadedFileInfo.lastWriteTime;
        m_syncHistory[file.name].cloudTime = file.lastWriteTime;
    } else {
        std::cerr << " [!] Ошибка скачивания: " << file.name << std::endl;
    }
}