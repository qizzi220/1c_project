# 🚀 CloudSync

<p align="center">
  <img src="https://img.shields.io/github/license/qizzi220/1c_project?style=for-the-badge&color=3399cc" alt="License">
  <img src="https://img.shields.io/badge/C%2B%2B-17%2F20-blue?style=for-the-badge&logo=c%2B%2B" alt="C++ Standards">
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey?style=for-the-badge&logo=linux" alt="Platform">
</p>

---

**CloudSync** — это высокопроизводительный кроссплатформенный модуль на C++ для автоматической синхронизации локальных папок с **Google Drive**. 

Проект оптимизирован для работы в качестве внешнего компонента **1С:Предприятие** или как независимая библиотека. Благодаря использованию Refresh Token, система поддерживает длительную работу без ручного вмешательства.

## ✨ Ключевые преимущества

*   **🔄 Бесконечная сессия:** Полная поддержка **Refresh Token** — забудьте о ручном обновлении Access Token каждые 60 минут.
*   **⚡ Скорость O(N):** Сверхбыстрое сравнение локальных и облачных метаданных через хеш-таблицы.
*   **🧠 Умная синхронизация:** Разрешение конфликтов по принципу **"Last Write Wins"**.
*   **🌍 Кроссплатформенность:** Работает "из коробки" на Linux и Windows.

---

## 📂Структура проекта
```
.
├── CMakeLists.txt              # Главный скрипт сборки всего проекта
├── main.cpp                    # Точка входа в приложение (инициализация и запуск)
├── config.json                 # Файл конфигурации (токены OAuth2, пути, история синхронизации)
│
├── include/                    # Заголовочные файлы (.h) - интерфейсы модулей
│   ├── CloudApi.h              # Сетевой уровень (работа с Google Drive API, curl, OAuth2)
│   ├── SyncManager.h           # Сердце проекта (логика дельт и координация синхронизации)
│   ├── LocalFolder.h           # Модуль сканирования локальных файлов
│   ├── FileInfo.h              # Структуры метаданных файлов и кроссплатформенный анализатор ОС
│   ├── CloudSync.h             # Общий заголовочный файл для удобного импорта библиотеки
│   └── nlohmann/               # Сторонняя библиотека для работы с JSON
│
├── src/                        # Исходный код (.cpp) - реализация логики
│   ├── CloudApi.cpp            # Реализация сетевых запросов и refresh токенов
│   ├── SyncManager.cpp         # Логика сравнения таймстампов и разрешения дельт
│   ├── LocalFolder.cpp         # Сканирование директорий на диске
│   └── FileInfo.cpp            # Кроссплатформенный сбор метаданных через stat ОС
│
├── tests/                      # Модульное тестирование (Google Test / Google Mock)
│   ├── CMakeLists.txt          # Скрипт сборки тестового бинарника
│   ├── test_syncmanager.cpp    # Тесты логики синхронизации с использованием MockCloudApi
│   ├── test_fileinfo.cpp       # Тесты анализатора файлов в изолированной песочнице
│   ├── test_cloudapi.cpp       # Тесты сетевого уровня
│   ├── test_localfolder.cpp    # Тесты сканирования папок
│   └── test_main.cpp           # Общие интеграционные тесты
│
├── CloudFiles/                 # Локальная рабочая папка по умолчанию (сюда синхронизируются файлы)
├── my_test_sync/               # Дополнительная папка для ручных проверок
├── диаграммы/                  # Архитектурная документация проекта (UML-схемы)
├── LICENSE                     # Юридическая информация (MIT Лицензия)
└── README.md                   # Документация проекта
```

--- 

## 🛠 Зависимости

1.  **libCURL**: Сетевое взаимодействие.
    *   *Ubuntu:* `sudo apt install libcurl4-openssl-dev`
    *   *Arch:* `sudo pacman -S curl`
2.  **[nlohmann-json](https://github.com/nlohmann/json)**: (Включено в `include/nlohmann`).
3.  **CMake**: Версия 3.10+.

---

## 🔑 Настройка авторизации (Refresh Token)

Для работы в автоматическом режиме вам необходимо настроить OAuth2 в Google Cloud Console:

1.  **Создайте проект** в [Google Cloud Console](https://console.cloud.google.com/).
2.  **Включите Google Drive API**.
3.  **Создайте OAuth Client ID** (тип: Desktop App) и сохраните ваш `Client ID` и `Client Secret`.
4.  **Получите Refresh Token**:
    *   Используйте [OAuth Playground](https://developers.google.com/oauthplayground) или собственный скрипт авторизации.
    *   При авторизации убедитесь, что запрашивается доступ `https://www.googleapis.com/auth/drive`.
    *   Важно: установите параметр `access_type=offline`, чтобы Google выдал Refresh Token.

---

## ⚙️ Настройка конфигурационного файла (config.json)

Перед запуском приложения необходимо заполнить файл `config.json` в корневой директории проекта. Этот файл используется программой для автоматической авторизации и отслеживания истории изменений.

Создайте файл со следующей структурой и вставьте свои актуальные данные:

```json
{
  "client_id": "YOUR_GOOGLE_CLIENT_ID.apps.googleusercontent.com",
  "client_secret": "YOUR_GOOGLE_CLIENT_SECRET",
  "refresh_token": "YOUR_REFRESH_TOKEN",
}

---

## 💻 Пример использования

Теперь при инициализации API модуль сам управляет обновлением временных токенов:

```cpp
#include "include/CloudSync.h"

int main() {
    // Данные вашего приложения Google
    std::string clientId = "YOUR_CLIENT_ID";
    std::string clientSecret = "YOUR_CLIENT_SECRET";
    std::string refreshToken = "YOUR_REFRESH_TOKEN";

    // Инициализация API с поддержкой авто-обновления
    auto api = std::make_shared<CloudApi>(clientId, clientSecret, refreshToken);
    
    // Синхронизация папки
    SyncManager sync(api, "./my_sync_folder");

    if (sync.initialize("config.json")) {
        sync.startSync(); // Модуль сам обновит Access Token при необходимости
    }

    return 0;
}
```

---

## 🚀 Сборка и запуск

```bash
# 1. Клонирование
git clone https://github.com/qizzi220/1c_project.git
cd 1c_project

# 2. Сборка
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 3. Запуск
./build/cloudsync
```

---

## 🗺 План разработки (Roadmap)

| Статус | Задача |
| :---: | :--- |
| ✅ | Базовый SyncManager ($O(N)$) |
| ✅ | Интеграция с Google Drive |
| ✅ | **Автоматическое обновление токена (Refresh Token)** |
| 🏗️ | Поддержка **Yandex Disk** (WebDAV) |
| 📅 | Продвинутое логирование в файл |
| 📅 | Система кастомных ошибок для 1С |

---

## 📄 Лицензия

Распространяется под лицензией **MIT**. Подробности в файле [LICENSE](./LICENSE).

---
<p align="center">
  Сделано для C++ сообщества 🛠️
</p>

