## **CloudSync** — это высокопроизводительный кроссплатформенный модуль на C++ для автоматической синхронизации локальных папок с облачными хранилищами (Google Drive). Идеально подходит для интеграции с внешними компонентами 1С или как самостоятельная библиотека.

## Преимущества нашей библиотеки:
    *CloudSync обеспечивает высокую скорость. Сравнение файлов происходит за O(N) благодаря хеш-таблицам из модуля filesystem.
    *CloudSync является умным синхронизатором. Логика разрешения конфликтов "Last Write Wins"
    *CloudSync является кроссплатформенным приложением. Полная поддержка Linux и Windows благодаря filesystem

## Для наглядности процесса синхронизации используется следующая диаграмма поведения:
    TODO:добавить в readme диаграммы

## Для сборки проекта вам понадобятся:
    1. Библиотека CURL: для работы с сетевыми запросами.
        - *Linux:* `sudo pacman -S curl` или `sudo apt install libcurl4-openssl-dev`.
    2. Библиотека nlohmann-json: (уже включена в проект в папке `include/nlohmann`).
    3. CMAKE: версия 3.10 или выше.


## Быстрый старт (Сборка)

```bash
# Клонировать репозиторий
git clone https://github.com/qizzi220/1c_project.git
cd 1c_project

# Создать конфиг с твоими данными
echo '{"client_id":"...","client_secret":"...","refresh_token":"..."}' > config.json

# Собрать проект
cmake -B build
cmake --build build

# Запустить
./build/cloudsync
```

## Пример использования

```C++
#include "include/CloudSync.h"
#include <memory>

int main() {
    // Данные для инициализации (обычно подтягиваются из конфига)
    std::string clientId = "YOUR_CLIENT_ID";
    std::string clientSecret = "YOUR_SECRET";
    std::string refreshToken = "YOUR_REFRESH_TOKEN";
    std::string accessToken = ""; // Можно оставить пустым, API обновит его сам

    // Создаем API с поддержкой Refresh Token
    auto api = std::make_shared<CloudApi>(accessToken, clientId, clientSecret, refreshToken);
    
    // Инициализируем менеджер синхронизации
    SyncManager sync(api, "./my_sync_folder");

    // Загружаем историю из конфига и запускаем процесс
    sync.initialize("config.json");
    sync.startSync();

    // Сохраняем обновленные токены и историю в файл после работы
    sync.saveConfig("config.json");

    return 0;
}
```

## Настройка Google Drive

Для корректной работы библиотеки CloudSync необходимо выполнить следующие шаги в Google Cloud Console:

1. **Создать проект**: Перейдите в [Google Cloud Console](https://console.cloud.google.com/) и создайте новый проект.
2. **Включить API**: В разделе "Library" найдите и включите **Google Drive API**.
3. **Создать учетные данные**:
* Перейдите в раздел "Credentials" и создайте **OAuth 2.0 Client ID** (тип приложения: *Web application* или *Desktop app*).
* Сохраните полученные `client_id` и `client_secret`.
  
4. **Получить Refresh Token**:
* Используйте [OAuth 2.0 Playground](https://developers.google.com/oauthplayground/).
* Выберите Google Drive API v3 (область видимости `[https://www.googleapis.com/auth/drive.file](https://www.googleapis.com/auth/drive.file)`).
* После авторизации обменяйте код на токены. **Refresh Token** является постоянным и используется библиотекой для генерации новых сессионных токенов.



**Важно**: Благодаря реализованной поддержке Refresh Token, CloudSync автоматически обновляет сессию каждые 60 минут и сохраняет актуальное состояние в `config.json`. Вам больше не нужно вручную обновлять `access_token`.



## План разработки (Roadmap) / TODO

  Y  Реализация базового SyncManager (O(N)).

  Y  Поддержка Google Drive

  Y  Реализация автоматического обновления токена через Refresh Token.

  X  Добавление поддержки Yandex Disk (WebDAV).

  X  Логирование событий в файл.

  X  Тестирование

  X  Реализация кастомных ошибок(в краааанйем случае)

