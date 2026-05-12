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
    std::cout << "   Запуск системы CloudSync v1.2 (Refresh Support & Save Config)" << std::endl;
    std::cout << "========================================" << std::endl;

    // Начальные значения (используются только если config.json пуст или отсутствует)
    std::string accessToken = ""; 
    std::string clientId     = "";
    std::string clientSecret = "";
    std::string refreshToken = "";
    
    std::string configName = "config.json";
    fs::path localDirPath = fs::current_path() / "CloudFiles";

    try {
        // Пытаемся загрузить актуальные токены из конфига, если он существует
        std::ifstream configFile(configName);
        if (configFile.is_open()) {
            json j;
            configFile >> j;
            accessToken = ""; // Очищаем, так как CloudApi сам обновит его через Refresh Token
            clientId = j.value("client_id", clientId);
            clientSecret = j.value("client_secret", clientSecret);
            refreshToken = j.value("refresh_token", refreshToken);
        }

        auto googleApi = std::make_shared<CloudApi>(accessToken, clientId, clientSecret, refreshToken);
        SyncManager manager(googleApi, localDirPath);
        
        // Загружаем историю синхронизации и проверяем соединение
        manager.initialize(configName); 

        if (!googleApi->isConnectedStatus()) { 
            throw std::runtime_error("Ошибка подключения к облаку.");
        }

        // Запуск основного процесса
        manager.startSync();

        // Сохраняем состояние (новые токены и метки времени файлов)
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