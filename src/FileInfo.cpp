#include "../include/FileInfo.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <system_error>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _WIN32
#include <unistd.h>
#endif

namespace fs = std::filesystem;

FileInfo::FileInfo(const std::string& pathStr) {
    *this = FileAnalyzer::getDetails(fs::path(pathStr));
}

FileInfo FileAnalyzer::getDetails(const fs::path& p) {
    FileInfo info;
    std::error_code ec;

    info.exists = fs::exists(p, ec);
    if (!info.exists || ec) {
        info.exists = false;
        return info;
    }

    info.name = p.filename().string();
    info.extension = p.extension().string();
    info.fullPath = fs::absolute(p, ec);
    info.isDirectory = fs::is_directory(p, ec);
    
    if (p.has_parent_path()) {
        info.parentFolder = p.parent_path().filename().string();
    }

    if (!info.isDirectory) {
        info.size = fs::file_size(p, ec);
        if (ec) info.size = 0;
    } else {
        info.size = 0;
    }

    // Берем время напрямую у ОС через stat, чтобы не зависеть от багов file_clock в C++17
#if defined(_WIN32)
    struct _stat64 result;
    if (_wstat64(p.wstring().c_str(), &result) == 0) {
        info.lastWriteTime = result.st_mtime;
    } else {
        info.lastWriteTime = 0;
    }
#else
    struct stat result;
    if (stat(p.string().c_str(), &result) == 0) {
        info.lastWriteTime = result.st_mtime;
    } else {
        info.lastWriteTime = 0;
    }
#endif

    return info;
}

std::string FileInfo::getFormattedSize() const {
    if (isDirectory) return "<DIR>";
    
    double bytes = static_cast<double>(size);
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    
    while (bytes >= 1024 && i < 4) {
        bytes /= 1024;
        i++;
    }
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << bytes << " " << units[i];
    return ss.str();
}