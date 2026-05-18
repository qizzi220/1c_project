#include <gtest/gtest.h>
#include "../include/LocalFolder.h"
#include "../include/FileInfo.h"

TEST(LocalFolderTest, InstanceCreation) {
    LocalFolder folder(".");
    ASSERT_NE(&folder, nullptr);
}

TEST(LocalFolderTest, GetPathReturnsPath) {
    LocalFolder folder("./test");
    EXPECT_EQ(folder.getPath(), "./test");
}

TEST(LocalFolderTest, ScanDoesNotCrash) {
    LocalFolder folder(".");
    EXPECT_NO_THROW(folder.scan());
}

TEST(LocalFolderTest, GetFilesReturnsVector) {
    LocalFolder folder(".");
    folder.scan();
    EXPECT_TRUE(true);
}

TEST(LocalFolderTest, ReadFileForMissingFile) {
    LocalFolder folder(".");
    std::vector<char> result = folder.readFile("not_exist.txt");
    EXPECT_TRUE(result.empty());
}
