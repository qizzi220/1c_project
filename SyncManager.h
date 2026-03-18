#pragma once

#include "CloudApi.h"
#include <string>
#include <vector>
#include <FileInfo.h>

class SyncManager{
    private:
    std::string* local = LocalFolder;
    std::string* cloud = CloudApi; 
    int syncInterval = 60;
    
    public:
    void SyncManager(l: LocalFolder&, c: CloudApi) {}
    void executeSync() {}
    void setSyncInterval(seconds: int) {}
};