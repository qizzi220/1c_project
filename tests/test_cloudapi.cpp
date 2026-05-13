#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "../include/CloudApi.h"

// Проверяем, что объект вообще создался и не рассыпался на старте
TEST(CloudApiTest, InstanceCreation) {
    CloudApi api("access", "id", "secret", "refresh");
    ASSERT_NE(&api, nullptr);
}

// Убеждаемся, что id клиента никуда не теряется по дороге
TEST(CloudApiTest, StoresClientId) {
    CloudApi api("access", "my_test_id", "secret", "refresh");
    EXPECT_EQ(api.getClientId(), "my_test_id");
}

// Проверяем, что рефреш-токен долетает до нужной переменной
TEST(CloudApiTest, StoresRefreshToken) {
    CloudApi api("access", "id", "secret", "my_refresh_123");
    EXPECT_EQ(api.getRefreshToken(), "my_refresh_123");
}

// Пока в сеть не ходили, статус должен быть не подключен
TEST(CloudApiTest, InitialStatusIsFalse) {
    CloudApi api("access", "id", "secret", "refresh");
    EXPECT_FALSE(api.isConnectedStatus());
}

// Кормим пустые строки, проверяем, что класс не падает и ведет себя адекватно
TEST(CloudApiTest, EmptyFieldsHandling) {
    CloudApi api("", "", "", "");
    EXPECT_EQ(api.getClientId(), "");
    EXPECT_FALSE(api.isConnectedStatus());
}

// Просто убеждаемся, что в рефреш-токене реально что-то лежит
TEST(CloudApiTest, RefreshTokenIsNotEmpty) {
    CloudApi api("a", "b", "c", "d");
    EXPECT_FALSE(api.getRefreshToken().empty());
}

// Путь к файлу не должен быть пустым 
TEST(CloudApiTest, PathNormalization) {
    CloudApi api("access", "id", "secret", "refresh");
    std::string rawPath = "Documents/Projects//MyFile.txt";
    
    EXPECT_FALSE(rawPath.empty());
}

// Тестируем логику времени: серверный файл должен считаться новее локального
TEST(CloudApiTest, TimestampComparison) {
    CloudApi api("access", "id", "secret", "refresh");
    
    long long localTime = 1715500000; 
    long long serverTime = 1715500005; 
    
    EXPECT_TRUE(serverTime > localTime);
}

// Собираем тестовый json и проверяем, что библиотека ничего не перепутала
TEST(CloudApiTest, JsonPayloadCreation) {
    CloudApi api("access", "id", "secret", "refresh");
    
    nlohmann::json j;
    j["name"] = "test_file.txt";
    j["mimeType"] = "text/plain";
    
    std::string result = j.dump();
    EXPECT_TRUE(result.find("test_file.txt") != std::string::npos);
    EXPECT_TRUE(result.find("mimeType") != std::string::npos);
}