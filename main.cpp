#include "include/CloudSync.h"
#include <iostream>
#include <memory>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif

    std::cout << "========================================" << std::endl;
    std::cout << "   Запуск системы CloudSync v1.1 (Refresh Support)" << std::endl;
    std::cout << "========================================" << std::endl;

    //1. Access Token (может быть пустым, если есть Refresh Token)
    std::string accessToken = "ваш accesToken"; 
    
    // 2. Данные вашего приложения из Google Cloud Console
    std::string clientId     = "ваш clientId";
    std::string clientSecret = "ваш clienSecret";
    
    // 3. Вечный токен (полученный один раз в Playground)
    std::string refreshToken = "ваш refreshToken";


    fs::path localDirPath = fs::current_path() / "CloudFiles";

    try {
        // Создаем объект API с полным набором данных для авто-обновления
        // (Убедись, что обновил сигнатуру конструктора в CloudApi.h)
        auto googleApi = std::make_shared<CloudApi>(accessToken, clientId, clientSecret, refreshToken);

        // Инициализируем менеджер синхронизации
        SyncManager manager(googleApi, localDirPath);

        std::cout << "[Main] Инициализация менеджера..." << std::endl;
        manager.initialize("config.json");

        std::cout << "[Main] Начинаю синхронизацию..." << std::endl;
        std::cout << "[Main] Локальная папка: " << fs::absolute(localDirPath) << std::endl;

        // Метод connect() внутри теперь сам проверит токен и обновит его при необходимости
      //  if (!googleApi->connect()) {
      //      throw std::runtime_error("Не удалось установить соединение с Google Drive");
     //   }

        // Запускаем основной процесс
        manager.startSync();

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
