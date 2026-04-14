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

# Собрать проект
cmake -B build
cmake --build build

# Запустить тест
./build/cloudsync

```

## Пример использования
```C++
#include "include/CloudSync.h"

int main() {
    auto api = std::make_shared<CloudApi>("YOUR_ACCESS_TOKEN");
    SyncManager sync(api, "./my_sync_folder");

    sync.initialize("config.json");
    sync.startSync();

    return 0;
}
```

## Настройка Google drive

1.Создать проект в Google Cloud Console.

2.Включить Google Drive API.

3.Получить OAuth2 Access Token (через OAuth Playground для тестов). Внимание! Пока проект работает только с ACCESS TOKEN. В будущих версиях будет произведено расширение для Refresh Token. Помните,что Access токен "тухнет" каждые 60 минут

## План разработки (Roadmap) / TODO

  Y  Реализация базового SyncManager (O(N)).

  Y  Поддержка Google Drive

  X  Добавление поддержки Yandex Disk (WebDAV).

  X  Реализация автоматического обновления токена через Refresh Token.

  X  Логирование событий в файл.

  X  Тестирование

  X  Реализация кастомных ошибок(в краааанйем случае)

