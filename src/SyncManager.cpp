#include "../include/SyncManager.h"
#include <iostream>
#include <unordered_map>
#include <filesystem>
#include <stdexcept>
#include <fstream> 

namespace fs = std::filesystem;

SyncManager::SyncManager(std::shared_ptr<CloudApi> api, const std::filesystem::path& folderPath)
    : m_cloudApi(std::move(api)), m_localFolder(folderPath) 
{
}

// инициализация
void SyncManager::initialize(const std::string& configPath) {
    m_configPath = configPath; // Сохраняем путь для последующей записи
    std::cout << "[SyncManager] Инициализация. Конфиг: " << configPath << std::endl;
    
    if (!m_cloudApi) {
        throw std::runtime_error("Cloud API не инициализирован");
    }

    // Загрузка истории синхронизации из файла
    std::ifstream file(configPath);
    if (file.is_open()) {
        nlohmann::json j;
        file >> j;
        if (j.contains("sync_history")) {
            for (auto it = j["sync_history"].begin(); it != j["sync_history"].end(); ++it) {
                m_syncHistory[it.key()] = it.value(); // Восстанавливаем метки времени
            }
        }
    }

    // проверка соединения с облаком
    if (!m_cloudApi->connect()) {
        throw std::runtime_error("Не удалось подключиться к облаку. Проверьте токен.");
    }

    if (!fs::exists(m_localFolder.getPath())) {
        std::cout << "[SyncManager] Локальная папка не найдена. Создаю: " << m_localFolder.getPath() << std::endl;
        fs::create_directories(m_localFolder.getPath());
    }
}

void SyncManager::startSync() {
    std::cout << "[SyncManager] Начало процесса синхронизации..." << std::endl;

    m_localFolder.scan();
    auto localFiles = m_localFolder.getFiles();

    auto cloudFiles = m_cloudApi->getCloudFiles();

    // Создание мапы облачных файлов за О(1),что оптимизирует наш код
    std::unordered_map<std::string, FileInfo> cloudFilesMap;
    for (const auto& cloudFile : cloudFiles) {
        cloudFilesMap[cloudFile.name] = cloudFile;
    }

    for (const auto& local : localFiles) {
        auto it = cloudFilesMap.find(local.name);

        if (it == cloudFilesMap.end()) {
            std::cout << "[SyncManager] Новый локальный файл: " << local.name << std::endl;
            upload(local);
        } else {
            compareAndResolve(local, it->second);
            // Удаляем из мапы, чтобы в конце остались только те файлы, которых нет локально
            cloudFilesMap.erase(it);
        }
    }

    // все файлы, оставшиеся в мапе есть в облаке, но их нет на диске
    for (const auto& [name, cloudInfo] : cloudFilesMap) {
        std::cout << "[SyncManager] Новый файл в облаке: " << name << std::endl;
        download(cloudInfo);
    }

    std::cout << "[SyncManager] Синхронизация успешно завершена." << std::endl;
}

void SyncManager::compareAndResolve(const FileInfo& local, const FileInfo& cloud) {
    std::time_t lastKnownTime = 0;
    if (m_syncHistory.count(local.name)) {
        lastKnownTime = m_syncHistory[local.name]; // Берем время из базы
    }

    // Если время в облаке изменилось с момента последней записи в базе
    if (cloud.lastWriteTime > lastKnownTime) {
        std::cout << "[SyncManager] Файл в облаке обновился: " << cloud.name << std::endl;
        download(cloud);
    } 
    // Если локальный файл изменился с момента последней записи в базе
    else if (local.lastWriteTime > lastKnownTime) {
        std::cout << "[SyncManager] Локальный файл изменился: " << local.name << std::endl;
        upload(local);
    }
}

// Запись актуальных данных в config.json
void SyncManager::saveConfig(const std::string& configPath) {
    nlohmann::json j;
    
    // Получаем текущие данные авторизации из API
    j["client_id"] = m_cloudApi-> getClientId();
    j["client_secret"] = m_cloudApi-> getClientSecret();
    j["refresh_token"] = m_cloudApi-> getRefreshToken();

    // Сохраняем базу меток времени
    for (const auto& [name, time] : m_syncHistory) {
        j["sync_history"][name] = time;
    }

    std::ofstream file(configPath);
    if (file.is_open()) {
        file << j.dump(4); // Красивый вывод с отступами
        std::cout << "[SyncManager] Конфигурация и история сохранены в " << configPath << std::endl;
    }
}

// обертка над методом загрузки API
void SyncManager::upload(const FileInfo& file) {
    if (m_cloudApi->uploadFile(file)) {
        std::cout << " [↑] Загружено: " << file.name << std::endl;
        // После загрузки нужно обновить историю. 
        // В идеале получить точное время от сервера, но пока фиксируем локальное
        m_syncHistory[file.name] = std::time(nullptr); 
    } else {
        std::cerr << " [!] Ошибка загрузки: " << file.name << std::endl;
    }
}

// О/обертка над методом скачивания API
void SyncManager::download(const FileInfo& file) {
    // собираем кроссплатформенный путь: путь_к_папке / имя_файла
    fs::path destination = fs::path(m_localFolder.getPath()) / file.name;

    if (m_cloudApi->downloadFile(file.name, destination)) {
        std::cout << " [↓] Скачано: " << file.name << std::endl;
        // Обновляем базу времени тем значением, которое пришло из облака
        m_syncHistory[file.name] = file.lastWriteTime;
    } else {
        std::cerr << " [!] Ошибка скачивания: " << file.name << std::endl;
    }
}