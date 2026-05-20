#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <vector>
#include "../include/FileInfo.h"

namespace fs = std::filesystem;

class FileInfoTest : public ::testing::Test {
protected:
    fs::path testSandbox;

    void SetUp() override {
        testSandbox = fs::temp_directory_path() / "file_info_tests_sandbox";
        fs::create_directories(testSandbox);
    }

    void TearDown() override {
        fs::remove_all(testSandbox);
    }

    fs::path createTestFile(const std::string& name, size_t sizeInBytes) {
        fs::path filePath = testSandbox / name;
        std::ofstream out(filePath, std::ios::binary);
        if (sizeInBytes > 0) {
            std::vector<char> dummyData(sizeInBytes, 'A');
            out.write(dummyData.data(), sizeInBytes);
        }
        out.close();
        return filePath;
    }
};

// тест: поведение, если файл не существует
TEST_F(FileInfoTest, NonExistentFileHandledCorrectly) {
    fs::path fakePath = testSandbox / "ghost_file.dat";
    
    FileInfo info = FileAnalyzer::getDetails(fakePath);
    
    EXPECT_FALSE(info.exists);
}

// тест: сбор метаданных обычного файла
TEST_F(FileInfoTest, AnalyzeRegularFileSuccess) {
    std::string filename = "document.txt";
    size_t expectedSize = 42;
    fs::path filePath = createTestFile(filename, expectedSize);

    FileInfo info = FileAnalyzer::getDetails(filePath);

    ASSERT_TRUE(info.exists);
    EXPECT_FALSE(info.isDirectory);
    EXPECT_EQ(info.name, filename);
    EXPECT_EQ(info.extension, ".txt");
    EXPECT_EQ(info.size, expectedSize);
    EXPECT_EQ(info.parentFolder, testSandbox.filename().string());
    EXPECT_EQ(info.fullPath, fs::absolute(filePath));
    EXPECT_GT(info.lastWriteTime, 0);
}

// тест: сбор метаданных для директории
TEST_F(FileInfoTest, AnalyzeDirectorySuccess) {
    fs::path subDir = testSandbox / "sub_folder";
    fs::create_directory(subDir);

    FileInfo info = FileAnalyzer::getDetails(subDir);

    ASSERT_TRUE(info.exists);
    EXPECT_TRUE(info.isDirectory);
    EXPECT_EQ(info.name, "sub_folder");
    EXPECT_EQ(info.size, 0);
    EXPECT_TRUE(info.extension.empty());
}

// тест: форматирование размера в разные единицы измерения
TEST_F(FileInfoTest, FormattedSizeOutputsCorrectUnits) {
    FileInfo dirInfo;
    dirInfo.isDirectory = true;
    EXPECT_EQ(dirInfo.getFormattedSize(), "<DIR>");

    FileInfo bytesInfo;
    bytesInfo.isDirectory = false;
    bytesInfo.size = 512;
    EXPECT_EQ(bytesInfo.getFormattedSize(), "512.00 B");

    FileInfo kbInfo;
    kbInfo.isDirectory = false;
    kbInfo.size = 2560;
    EXPECT_EQ(kbInfo.getFormattedSize(), "2.50 KB");

    FileInfo mbInfo;
    mbInfo.isDirectory = false;
    mbInfo.size = 1024 * 1024 * 10.15; 
    EXPECT_EQ(mbInfo.getFormattedSize(), "10.15 MB");
}

// тест: инициализация объекта через конструктор со строкой пути
TEST_F(FileInfoTest, ConstructorFromStringWorks) {
    std::string filename = "constructor_test.log";
    fs::path filePath = createTestFile(filename, 10);

    FileInfo info(filePath.string());

    EXPECT_TRUE(info.exists);
    EXPECT_EQ(info.name, filename);
    EXPECT_EQ(info.size, 10);
}