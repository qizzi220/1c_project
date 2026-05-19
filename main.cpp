#include "include/CloudSync.h"
#include <fstream>
#include "include/nlohmann/json.hpp"

using json = nlohmann::json;

int main() {
    // 1. Читаем конфиг
    std::string configPath = "config.json";
    std::ifstream f(configPath);
    if (!f.is_open()) {
        return 1; 
    }
    json config = json::parse(f);

    // 2. Создаем API
    // Передаем 4 аргумента: (пустой_токен, id, secret, refresh)
    auto api = std::make_shared<CloudApi>(
        "", 
        config["client_id"].get<std::string>(),
        config["client_secret"].get<std::string>(),
        config["refresh_token"].get<std::string>()
    );

    // 3. Создаем менеджер
    SyncManager sync(api, config["sync_folder"].get<std::string>());

    // 4. Запускаем процесс
    // Передаем путь к конфигу в initialize
    sync.initialize(configPath);
    sync.startSync();

    return 0;
}