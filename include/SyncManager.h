#ifndef SYNC_MANAGER_H
#define SYNC_MANAGER_H

#include <memory>
#include <filesystem>
#include <map> 
#include <string>
#include "CloudApi.h"
#include "LocalFolder.h"
#include <nlohmann/json.hpp> 

// два времени облачное и локальное, чтобы отслеживать изменения и там, и там
struct SyncState {
    std::time_t localTime = 0;
    std::time_t cloudTime = 0;
};

class SyncManager {
public:
    SyncManager(std::shared_ptr<CloudApi> api, const std::filesystem::path& folderPath);

    void initialize(const std::string& configPath);
    void startSync();
    void saveConfig(const std::string& configPath);

private:
    void compareAndResolve(const FileInfo& local, const FileInfo& cloud);
    void upload(const FileInfo& file);
    void download(const FileInfo& file);

    std::shared_ptr<CloudApi> m_cloudApi;
    LocalFolder m_localFolder;

    // история синхронизации: имя файла -> локальное и облачное время
    std::map<std::string, SyncState> m_syncHistory;
    std::string m_configPath;
};

#endif