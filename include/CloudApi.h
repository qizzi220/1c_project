#ifndef CLOUD_API_H
#define CLOUD_API_H

#include <string>
#include <vector>
#include <filesystem>
#include <curl/curl.h>
#include "FileInfo.h"

namespace fs = std::filesystem;

class CloudApi {
public:
    CloudApi(const std::string& token);
    virtual ~CloudApi();

    bool connect();
    std::vector<FileInfo> getCloudFiles();
    bool uploadFile(const FileInfo& file);
    bool downloadFile(const std::string& fileName, const fs::path& destination);

private:
    std::string apiToken;
    bool isConnected;

    std::string sendRequest(const std::string& url, const std::string& method, 
                            const std::string& body = "", struct curl_slist* extraHeaders = nullptr);
    std::string getFileIdByName(const std::string& fileName);
    std::string escapeString(const std::string& value);
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

#endif