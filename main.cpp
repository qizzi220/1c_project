#include "include/CloudSync.h"
#include <iostream>
#include <memory>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

int main() {
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif

    std::cout << "========================================" << std::endl;
    std::cout << "   Запуск системы CloudSync v1.2" << std::endl;
    std::cout << "========================================" << std::endl;

    std::string configName = "config.json";

    try {
        std::ifstream configFile(configName);
        if (!configFile.is_open()) {
            throw std::runtime_error("Не удалось открыть файл конфигурации.");
        }

        json j;
        configFile >> j;
        configFile.close();

        if (!j.contains("client_id") || !j.contains("client_secret") || !j.contains("refresh_token")) {
            throw std::runtime_error("В конфиге отсутствуют обязательные данные OAuth2.");
        }

        fs::path localDirPath = fs::current_path() / "CloudFiles";
        if (j.contains("sync_folder")) {
            localDirPath = j["sync_folder"].get<std::string>();
        }

        auto googleApi = std::make_shared<CloudApi>(
            "", 
            j["client_id"].get<std::string>(), 
            j["client_secret"].get<std::string>(), 
            j["refresh_token"].get<std::string>()
        );
        
        SyncManager manager(googleApi, localDirPath);
        manager.initialize(configName); 

        manager.startSync();
        manager.saveConfig(configName);

        std::cout << "========================================" << std::endl;
        std::cout << "   Синхронизация завершена!" << std::endl;
        std::cout << "========================================" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << std::endl;
        std::cerr << "!!! КРИТИЧЕСКАЯ ОШИБКА !!!" << std::endl;
        std::cerr << "-> " << e.what() << std::endl;
        return 1;
    }

    return 0;
}