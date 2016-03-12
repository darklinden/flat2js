//
//  FileUtilEx.h
//
//  Created by darklinden on 11/7/14.
//
//

#ifndef __FileUtilEx_H__
#define __FileUtilEx_H__

#include "cocos2d.h"
#include <string>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include <utime.h>

#define ENABLE_ZIP 1

class FileUtilEx {
    
public:
    
#pragma mark - platforms
    
    // enum all files in folder
    static std::vector<std::string> contentOfFolder(const std::string& path);
    
    // resource path
    static std::string resPath();
    
    // for ios only
    static void skipiCloudBackup(const std::string& path);
    
#pragma mark - file attr
    
    static int64_t getFileLen(const std::string& filePath) {
        if (!cocos2d::FileUtils::getInstance()->isFileExist(filePath)) {
            return 0;
        }
        std::ifstream in(filePath, std::ifstream::ate | std::ifstream::binary);
        int64_t len = in.tellg();
        return len;
    }
    
    static int64_t getFileTime(const std::string& filePath) {
        if (!cocos2d::FileUtils::getInstance()->isFileExist(filePath)) {
            return 0;
        }
        struct stat attr;
        stat(filePath.c_str(), &attr);
        return attr.st_mtime;
    }
    
    static int64_t dosDateToTime(unsigned long ulDosDate) {
        const unsigned long uDate = (unsigned long)(ulDosDate>>16);
        struct tm ptm;
        
        ptm.tm_mday = (unsigned int)(uDate&0x1f) ;
        ptm.tm_mon =  (unsigned int)((((uDate)&0x1E0)/0x20)-1) ;
        ptm.tm_year = (unsigned int)(((uDate&0x0FE00)/0x0200)+1980) - 1900 ;
        
        ptm.tm_hour = (unsigned int) ((ulDosDate &0xF800)/0x800);
        ptm.tm_min =  (unsigned int) ((ulDosDate&0x7E0)/0x20) ;
        ptm.tm_sec =  (unsigned int) (2*(ulDosDate&0x1f)) ;
        
        auto time = mktime(&ptm);
        
        return time;
    }
    
    static int64_t setFileTime(const std::string& filePath, long long timestamp) {
        if (!cocos2d::FileUtils::getInstance()->isFileExist(filePath)) {
            return 0;
        }
        struct stat attr;
        
        stat(filePath.c_str(), &attr);
        
        struct utimbuf new_times;
        new_times.actime = attr.st_atime; /* keep atime unchanged */
        new_times.modtime = timestamp;    /* set mtime to current time */
        utime(filePath.c_str(), &new_times);
        
        return 0;
    }
    
#pragma mark - encrypt
    
    static std::string encrypt(const std::string& str) { return str; };
    
    static std::string decrypt(const std::string& str) { return str; };
    
#pragma mark - other
    
    // last path component
    static std::string fileName(const std::string& path)
    {
        std::string ret = path;
        auto pos1 = path.rfind("\\");
        auto pos2 = path.rfind("/");
        
        if (pos1 != path.npos && pos2 != path.npos) {
            auto pos = MAX(pos1, pos2);
            ret = path.substr(pos, path.length() - pos);
        }
        else if (pos1 == path.npos && pos2 != path.npos) {
            auto pos = pos2;
            ret = path.substr(pos, path.length() - pos);
        }
        else if (pos1 == path.npos && pos2 == path.npos) {
            auto pos = pos1;
            ret = path.substr(pos, path.length() - pos);
        }
        return ret;
    }
    
#if ENABLE_ZIP
    static bool unzip2Folder(const std::string &zip, const std::string &folderPath);
#endif
};

#endif /* defined(__FileUtilEx_H__) */
