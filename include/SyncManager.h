#ifndef SYNC_MANAGER_H
#define SYNC_MANAGER_H

#include <memory>
#include <filesystem>
#include <map> 
#include <string>
#include "CloudApi.h"
#include "LocalFolder.h"
#include <nlohmann/json.hpp> 

class SyncManager {
public:
    SyncManager(std::shared_ptr<CloudApi> api, const std::filesystem::path& folderPath);

    void initialize(const std::string& configPath);
    void startSync();
    
    // Метод для записи всех данных (токены + история) на диск
    void saveConfig(const std::string& configPath);

private:
    void compareAndResolve(const FileInfo& local, const FileInfo& cloud);
    void upload(const FileInfo& file);
    void download(const FileInfo& file);

    std::shared_ptr<CloudApi> m_cloudApi;
    LocalFolder m_localFolder;

    // База данных: имя файла -> время последней успешной синхронизации
    std::map<std::string, std::time_t> m_syncHistory;
    
    // Путь к конфигу, чтобы менеджер знал, куда сохраняться
    std::string m_configPath;
};

#endif