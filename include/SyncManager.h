#ifndef SYNC_MANAGER_H
#define SYNC_MANAGER_H

#include "FileInfo.h"
#include "CloudApi.h"   
#include "LocalFolder.h"
#include <vector>
#include <memory>
#include <string>

class SyncManager {
public:
    // передаем путь в конструктор LocalFolder
    SyncManager(std::shared_ptr<CloudApi> api, const std::string& folderPath)
        : m_cloudApi(std::move(api)), m_localFolder(folderPath) {}

    void initialize(const std::string& configPath);
    void startSync();

private:
    // константные ссылки для предотвращения лишнего копирования
    void compareAndResolve(const FileInfo& local, const FileInfo& cloud);
    void upload(const FileInfo& file);
    void download(const FileInfo& file);

    std::shared_ptr<CloudApi> m_cloudApi;
    LocalFolder m_localFolder;
};

#endif // SYNC_MANAGER_H