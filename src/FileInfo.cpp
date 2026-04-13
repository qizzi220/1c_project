#include "FileInfo.h"
#include <iomanip>
#include <sstream>


FileInfo::FileInfo(const std::string& pathStr) {
    *this = FileAnalyzer::getDetails(pathStr);
}

FileInfo FileAnalyzer::getDetails(const std::string& pathStr) {
    fs::path p(pathStr);
    FileInfo info;

    info.exists = fs::exists(p);
    if (!info.exists) return info;

    info.name = p.filename().string();
    info.extension = p.extension().string();
    info.fullPath = fs::absolute(p);
    info.isDirectory = fs::is_directory(p);
    info.parentFolder = p.parent_path().filename().string();
    
    std::error_code ec;
    info.size = info.isDirectory ? 0 : fs::file_size(p, ec);
    info.lastWriteTime = fs::last_write_time(p, ec);

    return info;
}

std::string FileInfo::getFormattedSize() const {
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
