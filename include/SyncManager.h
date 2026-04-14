#ifndef SYNC_MANAGER_H
#define SYNC_MANAGER_H

#include <memory>
#include <filesystem>
#include "CloudApi.h"
#include "LocalFolder.h"

class SyncManager {
public:
    SyncManager(std::shared_ptr<CloudApi> api, const std::filesystem::path& folderPath);

    void initialize(const std::string& configPath);
    void startSync();

private:
    void compareAndResolve(const FileInfo& local, const FileInfo& cloud);
    void upload(const FileInfo& file);
    void download(const FileInfo& file);

    std::shared_ptr<CloudApi> m_cloudApi;
    LocalFolder m_localFolder;
};

#endif