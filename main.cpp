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
    std::cout << "   Запуск системы CloudSync v1.0" << std::endl;
    std::cout << "========================================" << std::endl;

    //OAuth2 токен от Google Drive
    std::string token = "PLACEHOLDER";//"ВНИМАНИЕ ТУТ ВАШ ТОКЕН ОБЯЗАТЕЛЬНО УДАЛЯЙТЕ ЕГО В ЦЕЛЯХ ОЕБСПЕЧЕНИЯ БЕЗОПАСНОСТИ";

    fs::path localDirPath = fs::current_path() / "ВАШЕ НАЗВАНИЕ ПАПКИ В ДИРЕКТОРИИ ПРОЕКТА";

    try {
        // Создаем объект API
        auto googleApi = std::make_shared<CloudApi>(token);

        // Инициализируем менеджер синхронизации
        // Передаем API и путь к папке
        SyncManager manager(googleApi, localDirPath);

        std::cout << "[Main] Инициализация менеджера..." << std::endl;
        // Передаем путь к конфигу (пока просто строка)
        manager.initialize("config.json");

        std::cout << "[Main] Начинаю синхронизацию..." << std::endl;
        std::cout << "[Main] Локальная папка: " << fs::absolute(localDirPath) << std::endl;

        // Запускаем основной процесс
        manager.startSync();

        std::cout << "========================================" << std::endl;
        std::cout << "   Синхронизация успешно завершена!" << std::endl;
        std::cout << "========================================" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << std::endl;
        std::cerr << "!!! КРИТИЧЕСКАЯ ОШИБКА !!!" << std::endl;
        std::cerr << "-> " << e.what() << std::endl;
        return 1;
    }

    return 0;
}