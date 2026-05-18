#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "../include/LocalFolder.h"
#include "../include/FileInfo.h"

namespace fs = std::filesystem;

// Пустая папка
TEST(LocalFolderTest, EmptyFolderReturnsNoFiles) {
    fs::path testDir = fs::temp_directory_path() / "cloudsync_test_empty";
    fs::create_directory(testDir);
    
    LocalFolder folder(testDir);
    folder.scan();
    
    EXPECT_EQ(folder.getFiles().size(), 0);
    
    fs::remove_all(testDir);
}

// getFiles() возвращает правильные имена
TEST(LocalFolderTest, GetFilesReturnsCorrectNames) {
    fs::path testDir = fs::temp_directory_path() / "cloudsync_test_names";
    fs::create_directory(testDir);
    
    std::ofstream(testDir / "alpha.txt") << "a";
    std::ofstream(testDir / "beta.txt") << "b";
    
    LocalFolder folder(testDir);
    folder.scan();
    
    auto files = folder.getFiles();
    
    bool foundAlpha = false;
    bool foundBeta = false;
    
    for (const auto& f : files) {
        if (f.name == "alpha.txt") foundAlpha = true;
        if (f.name == "beta.txt") foundBeta = true;
    }
    
    EXPECT_TRUE(foundAlpha);
    EXPECT_TRUE(foundBeta);
    
    fs::remove_all(testDir);
}

// setPath() меняет папку и пересканирует
TEST(LocalFolderTest, SetPathChangesDirectory) {
    fs::path dir1 = fs::temp_directory_path() / "cloudsync_test_set1";
    fs::path dir2 = fs::temp_directory_path() / "cloudsync_test_set2";
    
    fs::create_directory(dir1);
    fs::create_directory(dir2);
    
    std::ofstream(dir1 / "only_in_dir1.txt") << "data";
    std::ofstream(dir2 / "only_in_dir2.txt") << "data";
    
    LocalFolder folder(dir1);
    folder.scan();
    EXPECT_EQ(folder.getFiles().size(), 1);
    
    folder.setPath(dir2);
    EXPECT_EQ(folder.getFiles().size(), 1);
    
    fs::remove_all(dir1);
    fs::remove_all(dir2);
}

// getPath() возвращает правильный путь
TEST(LocalFolderTest, GetPathReturnsCorrectPath) {
    fs::path testDir = fs::temp_directory_path() / "cloudsync_test_path";
    fs::create_directory(testDir);
    
    LocalFolder folder(testDir);
    
    EXPECT_EQ(folder.getPath(), testDir);
    
    fs::remove_all(testDir);
}

// readFile() правильно читает содержимое файла
TEST(LocalFolderTest, ReadFileReturnsCorrectContent) {
    fs::path testDir = fs::temp_directory_path() / "cloudsync_test_read";
    fs::create_directory(testDir);
    
    std::string expected = "Hello, CloudSync!";
    std::ofstream(testDir / "test.txt") << expected;
    
    LocalFolder folder(testDir);
    folder.scan();
    std::vector<char> content = folder.readFile("test.txt");
    std::string actual(content.begin(), content.end());
    
    EXPECT_EQ(actual, expected);
    
    fs::remove_all(testDir);
}
