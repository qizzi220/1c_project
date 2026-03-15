#pragma once

#include <string>
#include <vector>
#include "FileInfo.h"

class CloudApi {
private:
    std::string apiToken;
    std::string serverUrl;
    long timeout;
    bool isConnected;

public:
    CloudApi(const std::string& token);
    
    bool connect();
    bool uploadFile(const FileInfo& file);
    bool downloadFile(const std::string& fileName);
    std::vector<FileInfo> getCloudFiles();
};