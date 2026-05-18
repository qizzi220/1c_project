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
```
