#pragma once

#include <string>
#include <vector>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "FileInfo.h"

using json = nlohmann::json;

class CloudApi {
private:
    std::string apiToken;
    std::string serverUrl = "https://www.googleapis.com/drive/v3/files";
    std::string uploadUrl = "https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart";
    bool isConnected;

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    std::string sendRequest(const std::string& url, const std::string& method, const std::string& body = "", struct curl_slist* extraHeaders = nullptr);

public:
   CloudApi(const std::string& token);
    ~CloudApi();

    bool connect(); 
    bool uploadFile(const FileInfo& file);
    std::vector<FileInfo> getCloudFiles();

    //возможно bool deleteFile(const std::string& fileId);
};
