#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>
#include <nlohmann/json.hpp>

#include "../include/SyncManager.h" 

namespace fs = std::filesystem;
using ::testing::Return;
using ::testing::_;

// Mock-класс для API облака
class MockCloudApi : public CloudApi {
public:
    MockCloudApi() : CloudApi("dummy_token", "dummy_id", "dummy_secret", "dummy_refresh") {}

    MOCK_METHOD(bool, connect, (), (override));
    MOCK_METHOD(std::vector<FileInfo>, getCloudFiles, (), (override));
    MOCK_METHOD(bool, uploadFile, (const FileInfo& file), (override));
    MOCK_METHOD(bool, downloadFile, (const std::string& name, const std::filesystem::path& destination), (override));

    MOCK_METHOD(std::string, getClientId, (), (const, override));
    MOCK_METHOD(std::string, getClientSecret, (), (const, override));
    MOCK_METHOD(std::string, getRefreshToken, (), (const, override));
};

class SyncManagerTest : public ::testing::Test {
protected:
    fs::path tempDir;
    fs::path tempConfig;
    fs::path localFolder;
    std::shared_ptr<MockCloudApi> mockApi;

    void SetUp() override {
        tempDir = fs::temp_directory_path() / "sync_manager_tests";
        fs::create_directories(tempDir);
        
        tempConfig = tempDir / "config.json";
        localFolder = tempDir / "local_sync";
        
        mockApi = std::make_shared<MockCloudApi>();
    }

    void TearDown() override {
        fs::remove_all(tempDir);
    }

    void createDummyConfig(const nlohmann::json& content) {
        std::ofstream out(tempConfig);
        out << content.dump();
        out.close();
    }
};

// тест инициализации с отсутствующим API
TEST_F(SyncManagerTest, InitializeThrowsOnNullApi) {
    SyncManager manager(nullptr, localFolder);
    EXPECT_THROW(manager.initialize(tempConfig.string()), std::runtime_error);
}

// тест неуспешного подключения к облаку при инициализации
TEST_F(SyncManagerTest, InitializeThrowsOnConnectionFailure) {
    SyncManager manager(mockApi, localFolder);

    EXPECT_CALL(*mockApi, connect())
        .WillOnce(Return(false));

    EXPECT_THROW(manager.initialize(tempConfig.string()), std::runtime_error);
}

// успешная инициализация и создание локальной папки
TEST_F(SyncManagerTest, InitializeSuccessAndCreatesDirectory) {
    if (fs::exists(localFolder)) {
        fs::remove_all(localFolder);
    }

    SyncManager manager(mockApi, localFolder);

    EXPECT_CALL(*mockApi, connect())
        .WillOnce(Return(true));

    EXPECT_NO_THROW(manager.initialize(tempConfig.string()));
    EXPECT_TRUE(fs::exists(localFolder));
}

// тест синхронизации: загрузка нового локального файла в облако
TEST_F(SyncManagerTest, SyncUploadsNewLocalFile) {
    SyncManager manager(mockApi, localFolder);

    EXPECT_CALL(*mockApi, connect()).WillOnce(Return(true));
    manager.initialize(tempConfig.string());

    fs::create_directories(localFolder);
    std::ofstream(localFolder / "new_local.txt") << "content";

    EXPECT_CALL(*mockApi, getCloudFiles())
        .WillOnce(Return(std::vector<FileInfo>{})); 

    EXPECT_CALL(*mockApi, uploadFile(_))
        .WillOnce(Return(true));

    manager.startSync();
}

// тест синхронизации: скачивание нового файла из облака
TEST_F(SyncManagerTest, SyncDownloadsNewCloudFile) {
    SyncManager manager(mockApi, localFolder);

    EXPECT_CALL(*mockApi, connect()).WillOnce(Return(true));
    manager.initialize(tempConfig.string());

    // явное создание структуры для обхода проблем с приведением типов
    FileInfo cloudFile;
    cloudFile.name = "cloud_file.txt";
    cloudFile.lastWriteTime = 1000;
    
    std::vector<FileInfo> cloudFiles;
    cloudFiles.push_back(cloudFile);

    EXPECT_CALL(*mockApi, getCloudFiles())
        .WillOnce(Return(cloudFiles));

    EXPECT_CALL(*mockApi, downloadFile("cloud_file.txt", _))
        .WillOnce(Return(true));

    manager.startSync();
}

// тест синхронизации: обновление устаревшего локального файла (скачивание из облака)
TEST_F(SyncManagerTest, SyncDownloadsWhenCloudIsNewer) {
    nlohmann::json historyConfig = {
        {"sync_history", {{"test.txt", 100}}}
    };
    createDummyConfig(historyConfig);

    SyncManager manager(mockApi, localFolder);

    EXPECT_CALL(*mockApi, connect()).WillOnce(Return(true));
    manager.initialize(tempConfig.string());

    fs::create_directories(localFolder);
    std::ofstream(localFolder / "test.txt") << "local_data";

    FileInfo cloudFile;
    cloudFile.name = "test.txt";
    cloudFile.lastWriteTime = 200;

    std::vector<FileInfo> cloudFiles;
    cloudFiles.push_back(cloudFile);

    EXPECT_CALL(*mockApi, getCloudFiles())
        .WillOnce(Return(cloudFiles));

    EXPECT_CALL(*mockApi, downloadFile("test.txt", _))
        .WillOnce(Return(true));

    manager.startSync();
}

// тест синхронизации: обновление устаревшего облачного файла (загрузка локального)
TEST_F(SyncManagerTest, SyncUploadsWhenLocalIsNewer) {
    nlohmann::json historyConfig = {
        {"sync_history", {{"test.txt", 100}}}
    };
    createDummyConfig(historyConfig);

    SyncManager manager(mockApi, localFolder);

    EXPECT_CALL(*mockApi, connect()).WillOnce(Return(true));
    manager.initialize(tempConfig.string());

    fs::create_directories(localFolder);
    std::ofstream localFile(localFolder / "test.txt");
    localFile << "new_local_data";
    localFile.close();

    FileInfo cloudFile;
    cloudFile.name = "test.txt";
    cloudFile.lastWriteTime = 100;

    std::vector<FileInfo> cloudFiles;
    cloudFiles.push_back(cloudFile);

    EXPECT_CALL(*mockApi, getCloudFiles())
        .WillOnce(Return(cloudFiles));

    EXPECT_CALL(*mockApi, uploadFile(_))
        .WillOnce(Return(true));

    manager.startSync();
}

// тест сохранения конфигурации
TEST_F(SyncManagerTest, SaveConfigWritesDataCorrectly) {
    SyncManager manager(mockApi, localFolder);

    EXPECT_CALL(*mockApi, getClientId()).WillOnce(Return("my_client_id"));
    EXPECT_CALL(*mockApi, getClientSecret()).WillOnce(Return("my_client_secret"));
    EXPECT_CALL(*mockApi, getRefreshToken()).WillOnce(Return("my_refresh_token"));

    manager.saveConfig(tempConfig.string());

    std::ifstream file(tempConfig);
    ASSERT_TRUE(file.is_open());
    
    nlohmann::json j;
    file >> j;

    EXPECT_EQ(j["client_id"], "my_client_id");
    EXPECT_EQ(j["client_secret"], "my_client_secret");
    EXPECT_EQ(j["refresh_token"], "my_refresh_token");
}