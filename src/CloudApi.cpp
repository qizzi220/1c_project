#include "CloudApi.h"
#include <curl/curl.h>   
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

bool CloudApi::uploadFile(const FileInfo& file) {
    if (!file.exists || file.isDirectory) return false;

    std::ifstream ifs(file.fullPath, std::ios::binary);
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

    std::string url = "https://www.googleapis.com/upload/drive/v3/files?uploadType=media";
    std::string response = sendRequest(url, "POST", content, headers);
    
    return !response.empty() && response.find("error") == std::string::npos;
}

std::vector<FileInfo> CloudApi::getCloudFiles() {
    std::vector<FileInfo> cloudFiles;
    if (!isConnected && !connect()) return cloudFiles;
    std::string url = "https://www.googleapis.com/drive/v3/files?fields=files(id,name,size)";
    std::string response = sendRequest(url, "GET");

    try {
        auto data = json::parse(response);
        if (data.contains("files")) {
            for (auto& item : data["files"]) {
                FileInfo info;
                info.name = item["name"];
                // info.cloudId = item["id"]; // поменять fileinfo
                info.exists = true;
                if (item.contains("size")) {
                    info.size = std::stoull(item["size"].get<std::string>());
                }
                cloudFiles.push_back(info);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "JSON Parse Error: " << e.what() << std::endl;
    }

    return cloudFiles;
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
        
        if (method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)body.size());
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        
        if (headers != extraHeaders) curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "CURL Error: " << curl_easy_strerror(res) << std::endl;
            return "";
        }
    }
    return response;
}
