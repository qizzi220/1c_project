#include "../include/FileInfo.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <system_error>

namespace fs = std::filesystem;

// Конструктор
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

    // базовая информация
    info.name = p.filename().string();
    info.extension = p.extension().string();
    info.fullPath = fs::absolute(p, ec);
    info.isDirectory = fs::is_directory(p, ec);
    
    // родительская папка
    if (p.has_parent_path()) {
        info.parentFolder = p.parent_path().filename().string();
    }

    // размер файла
    if (!info.isDirectory) {
        info.size = fs::file_size(p, ec);
        if (ec) info.size = 0;
    } else {
        info.size = 0;
    }

    // --- ПРЕОБРАЗОВАНИЕ ВРЕМЕНИ (Кроссплатформенное для C++17) ---
    // В C++17 last_write_time возвращает file_time_type, который сложно сравнивать напрямую.
    // Мы конвертируем его в std::time_t (секунды с 1970 года).
    auto ftime = fs::last_write_time(p, ec);
    if (!ec) {
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
        );
        info.lastWriteTime = std::chrono::system_clock::to_time_t(sctp);
    } else {
        info.lastWriteTime = 0;
    }

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