#include "CloudApi.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

using json = nlohmann::json;

size_t CloudApi::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

CloudApi::CloudApi(const std::string& token) : apiToken(token), isConnected(false) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

CloudApi::~CloudApi() {
    curl_global_cleanup();
}

bool CloudApi::connect() {
    std::string res = sendRequest("https://www.googleapis.com/drive/v3/about?fields=user", "GET");
    if (!res.empty() && res.find("error") == std::string::npos) {
        isConnected = true;
        return true;
    }
    return false;
}

std::string CloudApi::getFileIdByName(const std::string& fileName) {
    std::string url = "https://www.googleapis.com/drive/v3/files?q=name='" + fileName + "' and trashed=false&fields=files(id)";
    std::string response = sendRequest(url, "GET");
    
    if (response.empty()) return "";

    try {
        auto data = json::parse(response);
        if (data.contains("files") && !data["files"].empty()) {
            return data["files"][0]["id"].get<std::string>();
        }
    } catch (...) {}

    return "";
}

std::vector<FileInfo> CloudApi::getCloudFiles() {
    std::vector<FileInfo> files;
    if (!isConnected && !connect()) return files;

    std::string url = "https://www.googleapis.com/drive/v3/files?fields=files(name,size)";
    std::string response = sendRequest(url, "GET");

    try {
        auto data = json::parse(response);
        for (auto& item : data["files"]) {
            FileInfo info;
            info.name = item["name"].get<std::string>();
            if (item.contains("size")) {
                info.size = std::stoull(item["size"].get<std::string>());
            }
            info.exists = true;
            files.push_back(info);
        }
    } catch (...) {
        std::cerr << "[CloudApi] Ошибка парсинга списка файлов" << std::endl;
    }
    return files;
}

bool CloudApi::uploadFile(const FileInfo& file) {
    std::string cloudId = getFileIdByName(file.name);
    
    std::ifstream ifs(file.fullPath, std::ios::binary);
    if (!ifs.is_open()) return false;
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    std::string url;
    std::string method;

    if (cloudId.empty()) {
        url = "https://www.googleapis.com/upload/drive/v3/files?uploadType=media";
        method = "POST";
    } else {
        url = "https://www.googleapis.com/upload/drive/v3/files/" + cloudId + "?uploadType=media";
        method = "PATCH";
    }

    std::string response = sendRequest(url, method, content);
    return !response.empty() && response.find("error") == std::string::npos;
}

std::string CloudApi::sendRequest(const std::string& url, const std::string& method, const std::string& body, struct curl_slist* extraHeaders) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        struct curl_slist* headers = extraHeaders;
        std::string auth = "Authorization: Bearer " + apiToken;
        headers = curl_slist_append(headers, auth.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());

        if (!body.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)body.size());
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        
        if (headers != extraHeaders) curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "[CloudApi] CURL Error: " << curl_easy_strerror(res) << std::endl;
            return "";
        }
    }
    return response;
}
