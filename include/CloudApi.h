#pragma once

#include <string>
#include <vector>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "FileInfo.h"

class CloudApi {
private:
    std::string apiToken;
    bool isConnected;

    std::string sendRequest(const std::string& url, 
                           const std::string& method, 
                           const std::string& body = "", 
                           struct curl_slist* extraHeaders = nullptr);

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

public:
    CloudApi(const std::string& token);
    ~CloudApi();

    bool connect();
    std::vector<FileInfo> getCloudFiles();
    bool uploadFile(FileInfo& file); 
};
