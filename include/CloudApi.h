#pragma once

#include <string>
#include <vector>
#include "FileInfo.h"

class CloudApi {
private:
    std::string apiToken;
    bool isConnected;

    std::string sendRequest(const std::string& url, const std::string& method, const std::string& body = "");
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

    std::string getFileIdByName(const std::string& fileName);

public:
    CloudApi(const std::string& token);
    ~CloudApi();

    bool connect();
    std::vector<FileInfo> getCloudFiles();
    bool uploadFile(const FileInfo& file); 
    bool downloadFile(const std::string& fileName, const std::string& downloadPath);
};
