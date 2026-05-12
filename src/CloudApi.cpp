#include "../include/CloudApi.h"
#include "../include/nlohmann/json.hpp"
#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

using json = nlohmann::json;

// Вспомогательная функция для парсинга времени из формата Google (RFC3339 / ISO 8601)
// Google присылает время как "2023-10-27T10:00:00.000Z"
static std::time_t parseGoogleTime(const std::string& timeStr) {
    std::tm tm = {};
    std::istringstream ss(timeStr);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    // Используем timegm для получения чистого UTC timestamp (исправляет смещение часовых поясов)
    return (ss.fail()) ? 0 : timegm(&tm); 
}

CloudApi::CloudApi(const std::string& token, 
                   const std::string& cId, 
                   const std::string& cSecret, 
                   const std::string& rToken) 
    : apiToken(token), clientId(cId), clientSecret(cSecret), refreshToken(rToken), isConnected(false) 
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

CloudApi::~CloudApi() {
    curl_global_cleanup();
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
    char* output = curl_easy_escape(curl, value.c_str(), (int)value.length());
    std::string result(output);
    curl_free(output);
    curl_easy_cleanup(curl);
    return result;
}

// Обновление access_token через refresh_token, когда старый истёк
bool CloudApi::refreshAccessToken() {
    std::string url = "https://oauth2.googleapis.com/token";
    
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    // кодируем каждый параметр
    char* c_id = curl_easy_escape(curl, clientId.c_str(), 0);
    char* c_sec = curl_easy_escape(curl, clientSecret.c_str(), 0);
    char* r_tok = curl_easy_escape(curl, refreshToken.c_str(), 0);

    std::string body = "client_id=" + std::string(c_id) + 
                       "&client_secret=" + std::string(c_sec) + 
                       "&refresh_token=" + std::string(r_tok) + 
                       "&grant_type=refresh_token";

    // Освобождаем память после кодирования
    curl_free(c_id); curl_free(c_sec); curl_free(r_tok);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

    // Выполняем запрос
    std::string response = sendRequest(url, "POST", body, headers);
    curl_slist_free_all(headers);

    try {
        auto data = json::parse(response);
        if (data.contains("access_token")) {
            this->apiToken = data["access_token"].get<std::string>();
            
            // Если Google прислал новый refresh_token, сохраняем его
            if (data.contains("refresh_token")) {
                this->refreshToken = data["refresh_token"].get<std::string>();
            }

            std::cout << "[CloudApi] Токен успешно обновлен!" << std::endl;
            curl_easy_cleanup(curl);
            return true;
        }
    } catch (...) {}

    curl_easy_cleanup(curl);
    return false;
}

// Поиск или создание рабочей папки приложения в корне Диска
bool CloudApi::setupRootFolder(const std::string& folderName) {
    // Ищем папку по имени и MIME-типу
    std::string query = "name='" + folderName + "' and mimeType='application/vnd.google-apps.folder' and trashed=false";
    std::string url = "https://www.googleapis.com/drive/v3/files?q=" + escapeString(query);
    
    std::string res = sendRequest(url, "GET");
    if (res.empty()) return false;

    auto data = json::parse(res);
    if (data.contains("files") && !data["files"].empty()) {
        rootFolderId = data["files"][0]["id"].get<std::string>();
        std::cout << "[CloudApi] Найдена папка: " << folderName << " (ID: " << rootFolderId << ")" << std::endl;
        return true;
    }

    // Если папки нет,то создаем её через метаданные json
    json meta = {{"name", folderName}, {"mimeType", "application/vnd.google-apps.folder"}};
    struct curl_slist* headers = curl_slist_append(nullptr, "Content-Type: application/json");
    std::string createRes = sendRequest("https://www.googleapis.com/drive/v3/files", "POST", meta.dump(), headers);
    curl_slist_free_all(headers);

    auto newData = json::parse(createRes);
    if (newData.contains("id")) {
        rootFolderId = newData["id"].get<std::string>();
        std::cout << "[CloudApi] Создана новая папка: " << folderName << std::endl;
        return true;
    }
    return false;
}

bool CloudApi::connect() {
    std::cout << "[CloudApi] Проверка соединения..." << std::endl;
    
    std::string res = "";
    bool needsRefresh = apiToken.empty();

    // 1) Проверяем текущий токен, если он есть
    if (!needsRefresh) {
        res = sendRequest("https://www.googleapis.com/drive/v3/about?fields=user", "GET");
        // Если Google ответил ошибкой, значит токен протух
        if (res.empty() || res.find("error") != std::string::npos) {
            needsRefresh = true;
        }
    }

    // 2) Если токен плохой или его нет, то обновляем
    if (needsRefresh) {
        std::cout << "[CloudApi] Рабочий токен отсутствует или просрочен. Обновление..." << std::endl;
        if (!refreshAccessToken()) {
            std::cerr << "[CloudApi] Ошибка: Не удалось обновить токен. Проверьте сеть." << std::endl;
            return false;
        }
        // Проверяем связь с новым токеном
        res = sendRequest("https://www.googleapis.com/drive/v3/about?fields=user", "GET");
    }

    // 3) Финальная проверка и вход в папку
    if (!res.empty() && res.find("error") == std::string::npos) {
        isConnected = true;
        std::cout << "[CloudApi] Авторизация подтверждена." << std::endl;
        return setupRootFolder("CloudSync_Storage");
    }
    
    std::cerr << "[CloudApi] Ошибка: Авторизация отклонена сервером Google." << std::endl;
    return false;
}

std::string CloudApi::getFileIdByName(const std::string& fileName) {
    // Теперь ищем файл строго внутри нашей рабочей папки (parents)
    std::string query = "name='" + fileName + "' and '" + rootFolderId + "' in parents and trashed=false";
    std::string url = "https://www.googleapis.com/drive/v3/files?q=" + escapeString(query);
    
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
    if (!isConnected) return files;

    // запрашиваем список файлов, находящихся только в нашей папке
    std::string query = "'" + rootFolderId + "' in parents and trashed=false";
    std::string url = "https://www.googleapis.com/drive/v3/files?q=" + escapeString(query) + "&fields=files(id,name,size,modifiedTime)";
    
    std::string response = sendRequest(url, "GET");
    if (response.empty()) return files;

    try {
        auto data = json::parse(response);
        for (auto& item : data["files"]) {
            FileInfo info;
            info.name = item.value("name", "unknown");
            
            // запрашиваем имя, размер и время модификации
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

    if (cloudId.empty()) {
        // Создаем метаданные для нового файла, указывая родительскую папку
        json meta = { {"name", file.name}, {"parents", {rootFolderId}} };
        struct curl_slist* headers = curl_slist_append(nullptr, "Content-Type: application/json");
        std::string res = sendRequest("https://www.googleapis.com/drive/v3/files", "POST", meta.dump(), headers);
        curl_slist_free_all(headers);
        
        try {
            auto data = json::parse(res);
            cloudId = data.value("id", "");
        } catch (...) { return false; }
    }

    if (cloudId.empty()) return false;

    // читаем контент локального файла
    std::ifstream ifs(file.fullPath, std::ios::binary);
    if (!ifs.is_open()) return false;
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    // Обновляем содержимое (uploadType=media)
    std::string url = "https://www.googleapis.com/upload/drive/v3/files/" + cloudId + "?uploadType=media";
    struct curl_slist* headers = curl_slist_append(nullptr, "Content-Type: application/octet-stream");
    std::string response = sendRequest(url, "PATCH", content, headers);
    curl_slist_free_all(headers);

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

    // Открываем файл для записи
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
        
        // копируем дополнительные заголовки 
        if (extraHeaders) {
            for (auto* it = extraHeaders; it; it = it->next) {
                headers = curl_slist_append(headers, it->data);
            }
        }
        
        headers = curl_slist_append(headers, ("Authorization: Bearer " + apiToken).c_str());

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
        
        // таймаут между реквестами, чтобы сервер не подумал, что мы боты
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