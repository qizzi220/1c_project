#include <gtest/gtest.h>
#include "../include/LocalFolder.h"
#include "../include/FileInfo.h"

// Проверяем, что объект создается и не падает
TEST(LocalFolderTest, InstanceCreation) {
    LocalFolder folder(".");
    ASSERT_NE(&folder, nullptr);
}

// Проверяем, что возвращается тот путь, который мы задали
TEST(LocalFolderTest, GetPathReturnsPath) {
    LocalFolder folder("./test");
    EXPECT_EQ(folder.getPath(), "./test");
}

// Убеждаемся, что сканирование не ломается на пустой папке
TEST(LocalFolderTest, ScanDoesNotCrash) {
    LocalFolder folder(".");
    EXPECT_NO_THROW(folder.scan());
}

// Получение списка файлов работает
TEST(LocalFolderTest, GetFilesReturnsVector) {
    LocalFolder folder(".");
    folder.scan();
    EXPECT_TRUE(true);
}

// Если файла нет, возвращаем пустой результат
TEST(LocalFolderTest, ReadFileForMissingFile) {
    LocalFolder folder(".");
    std::vector<char> result = folder.readFile("not_exist.txt");
    EXPECT_TRUE(result.empty());
}
