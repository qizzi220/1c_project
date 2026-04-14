#include "../include/CloudApi.h"
#include "../include/nlohmann/json.hpp"
#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;
namespace fs = std::filesystem;

// Вспомогательная функция для парсинга времени из формата Google (RFC3339 / ISO 8601)
// Google присылает время как "2023-10-27T10:00:00.000Z"
static std::time_t parseGoogleTime(const std::string& timeStr) {
    std::tm tm = {};
    std::istringstream ss(timeStr);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (ss.fail()) return 0;
    return std::mktime(&tm);
}

// запись ответа от сервера в строку
size_t CloudApi::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    if (userp) {
        ((std::string*)userp)->append((char*)contents, totalSize);
    }
    return totalSize;
}

// кодирование строк для URL
std::string CloudApi::escapeString(const std::string& value) {
    CURL* curl = curl_easy_init();
    if (!curl) return value;
    char* output = curl_easy_escape(curl, value.c_str(), static_cast<int>(value.length()));
    std::string result(output);
    curl_free(output);
    curl_easy_cleanup(curl);
    return result;
}

CloudApi::CloudApi(const std::string& token) : apiToken(token), isConnected(false) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

CloudApi::~CloudApi() {
    curl_global_cleanup();
}

bool CloudApi::connect() {
    // проверка токена через запрос информации об аккаунте
    std::string res = sendRequest("https://www.googleapis.com/drive/v3/about?fields=user", "GET");
    if (!res.empty() && res.find("error") == std::string::npos) {
        isConnected = true;
        return true;
    }
    isConnected = false;
    return false;
}

std::string CloudApi::getFileIdByName(const std::string& fileName) {
    // кодируем имя файла, так как в нем могут быть кавычки или пробелы
    std::string query = escapeString("name='" + fileName + "' and trashed=false");
    std::string url = "https://www.googleapis.com/drive/v3/files?q=" + query + "&fields=files(id)";
    
    std::string response = sendRequest(url, "GET");
    if (response.empty()) return "";

    try {
        auto data = json::parse(response);
        if (data.contains("files") && !data["files"].empty()) {
            return data["files"][0]["id"].get<std::string>();
        }
    } catch (const std::exception& e) {
        std::cerr << "[CloudApi] JSON Error in getFileIdByName: " << e.what() << std::endl;
    }

    return "";
}

std::vector<FileInfo> CloudApi::getCloudFiles() {
    std::vector<FileInfo> files;
    if (!isConnected && !connect()) return files;

    // запрашиваем имя, размер и время модификации
    std::string url = "https://www.googleapis.com/drive/v3/files?fields=files(id,name,size,modifiedTime)";
    std::string response = sendRequest(url, "GET");

    if (response.empty()) return files;

    try {
        auto data = json::parse(response);
        for (auto& item : data["files"]) {
            FileInfo info;
            info.name = item.value("name", "unknown");
            
            if (item.contains("size")) {
                info.size = std::stoull(item["size"].get<std::string>());
            } else {
                info.size = 0;
            }

            if (item.contains("modifiedTime")) {
                info.lastWriteTime = parseGoogleTime(item["modifiedTime"].get<std::string>());
            }

            info.exists = true;
            files.push_back(info);
        }
    } catch (const std::exception& e) {
        std::cerr << "[CloudApi] Ошибка парсинга списка файлов: " << e.what() << std::endl;
    }
    return files;
}

bool CloudApi::uploadFile(const FileInfo& file) {
    std::string cloudId = getFileIdByName(file.name);
    
    // читаем контент файла
    std::ifstream ifs(file.fullPath, std::ios::binary);
    if (!ifs.is_open()) {
        std::cerr << "[CloudApi] Не удалось открыть локальный файл: " << file.fullPath << std::endl;
        return false;
    }
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    std::string url;
    std::string method;

    if (cloudId.empty()) {
        // если файла нет — создаем новый (POST)
        // для простоты используется multipart/related или просто заголовок имени в параметрах
        url = "https://www.googleapis.com/upload/drive/v3/files?uploadType=media";
        // Примечание: Чтобы задать имя НОВОМУ файлу, Google требует сложный запрос.
        // Для этой реализации имя лучше передавать через метаданные, но для краткости оставим упрощенно.
        method = "POST";
    } else {
        // Если файл есть — патчим содержимое
        url = "https://www.googleapis.com/upload/drive/v3/files/" + cloudId + "?uploadType=media";
        method = "PATCH";
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

    std::string response = sendRequest(url, method, content, headers);
    
    if (headers) curl_slist_free_all(headers);
    
    return !response.empty() && response.find("error") == std::string::npos;
}

bool CloudApi::downloadFile(const std::string& fileName, const fs::path& destination) {
    std::string fileId = getFileIdByName(fileName);
    if (fileId.empty()) {
        std::cerr << "[CloudApi] Файл не найден в облаке: " << fileName << std::endl;
        return false;
    }

    std::string url = "https://www.googleapis.com/drive/v3/files/" + fileId + "?alt=media";
    
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    // Открываем файл для записи. .string() делает путь совместимым.
    FILE* fp = fopen(destination.string().c_str(), "wb");
    if (!fp) {
        std::cerr << "[CloudApi] Не удалось создать локальный файл: " << destination << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    std::string auth = "Authorization: Bearer " + apiToken;
    struct curl_slist* headers = curl_slist_append(nullptr, auth.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL); // По умолчанию пишет в WRITEDATA
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp); 
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        std::cerr << "[CloudApi] Download error: " << curl_easy_strerror(res) << std::endl;
    }

    fclose(fp);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return res == CURLE_OK;
}

std::string CloudApi::sendRequest(const std::string& url, const std::string& method, const std::string& body, struct curl_slist* extraHeaders) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        struct curl_slist* headers = nullptr;
        
        // копируем extraHeaders если они есть
        if (extraHeaders) {
            struct curl_slist* temp = extraHeaders;
            while(temp) {
                headers = curl_slist_append(headers, temp->data);
                temp = temp->next;
            }
        }
        
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
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        // таймаут между реквестами,чтобы сервер не подумал,что мы боты
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

        CURLcode res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            std::cerr << "[CloudApi] CURL Error: " << curl_easy_strerror(res) << " (URL: " << url << ")" << std::endl;
            response = "";
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    return response;
}