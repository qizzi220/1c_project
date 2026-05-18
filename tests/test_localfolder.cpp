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
