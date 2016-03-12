//
//  FileUtilEx.cpp
//
//  Created by darklinden on 11/7/14.
//
//

#include "FileUtilEx.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS

#if ENABLE_ZIP
#define BUFFER_SIZE    8192
#define MAX_FILENAME   512
#include "unzip/unzip.h"

#endif

std::vector<std::string> FileUtilEx::contentOfFolder(const std::string& path)
{
    std::vector<std::string> ret;
    ret.clear();
    
    NSArray* array = [[NSFileManager defaultManager]
                      contentsOfDirectoryAtPath:[NSString stringWithUTF8String:path.c_str()]
                      error:nil];
    
    for (int i = 0; i < array.count; i++) {
        ret.push_back(std::string([array[i] UTF8String]));
    }
    
    return ret;
}

std::string FileUtilEx::resPath()
{
    NSString* path = [[NSBundle mainBundle] resourcePath];
    std::string ret(path.UTF8String);
    while (ret.substr(ret.length() - 1) == "/") {
        ret = ret.substr(0, ret.length() - 1);
    }
    ret += "/";
    return ret;
}

void FileUtilEx::skipiCloudBackup(const std::string& path)
{
    NSString* filePath = [NSString stringWithUTF8String:path.c_str()];
    
    if (![[NSFileManager defaultManager] fileExistsAtPath:filePath]) {
        return;
    }
    
    NSURL* URL = [NSURL fileURLWithPath:filePath];
    if (URL) {
        [URL setResourceValue:[NSNumber numberWithBool:YES]
                       forKey:NSURLIsExcludedFromBackupKey
                        error:nil];
    }
}

bool FileUtilEx::copyFolder(bool FileUtilEx::unzip2Folder(const std::string &zip, const std::string &folderPath)
{
}

#if ENABLE_ZIP
std::string basename(const std::string& path)
{
    size_t found = path.find_last_of("/\\");
    
    if (std::string::npos != found)
    {
        return path.substr(0, found);
    }
    else
    {
        return path;
    }
}

bool FileUtilEx::unzip2Folder(const std::string &zip, const std::string &folderPath)
{
    std::string fzipPath = zip;
    
    // Find root path for zip file
    // Open the zip file
    unzFile zipfile = cocos2d::unzOpen(fzipPath.c_str());
    if (!zipfile) {
        CCLOG("FileUtilEx::unzip2Folder: can not open zipped src file %s\n", fzipPath.c_str());
        return false;
    }
    
    // Get info about the zip file
    cocos2d::unz_global_info global_info;
    if (unzGetGlobalInfo(zipfile, &global_info) != UNZ_OK) {
        CCLOG("FileUtilEx::unzip2Folder: can not read file global info of %s\n", zip.c_str());
        cocos2d::unzClose(zipfile);
        return false;
    }
    
    // Buffer to hold data read from the zip file
    char readBuffer[BUFFER_SIZE];
    // Loop to extract all files.
    uLong i;
    for (i = 0; i < global_info.number_entry; ++i) {
        // Get info about current file.
        cocos2d::unz_file_info fileInfo;
        char fileName[MAX_FILENAME];
        if (unzGetCurrentFileInfo(zipfile,
                                  &fileInfo,
                                  fileName,
                                  MAX_FILENAME,
                                  NULL,
                                  0,
                                  NULL,
                                  0) != UNZ_OK)
        {
            CCLOG("FileUtilEx::unzip2Folder: can not read compressed file info\n");
            cocos2d::unzClose(zipfile);
            return false;
        }
        
        const std::string fullPath = folderPath + fileName;
        // CCLOG("FileUtilEx::unzip2Folder: extract file %s\n", fullPath.c_str());
        
        // Check if this entry is a directory or a file.
        const size_t filenameLength = strlen(fileName);
        if (fileName[filenameLength - 1] == '/') {
            //There are not directory entry in some case.
            //So we need to create directory when decompressing file entry
            if (!cocos2d::FileUtils::getInstance()->createDirectory(basename(fullPath))) {
                // Failed to create directory
                CCLOG("FileUtilEx::unzip2Folder: can not create directory %s\n", fullPath.c_str());
                cocos2d::unzClose(zipfile);
                return false;
            }
        }
        else {
            // Entry is a file, so extract it.
            // Open current file.
            if (cocos2d::unzOpenCurrentFile(zipfile) != UNZ_OK) {
                CCLOG("FileUtilEx::unzip2Folder: can not extract file %s\n", fileName);
                cocos2d::unzClose(zipfile);
                return false;
            }
            
            if (!cocos2d::FileUtils::getInstance()->createDirectory(basename(fullPath))) {
                // Failed to create directory
                CCLOG("FileUtilEx::unzip2Folder: can not create directory %s\n", fullPath.c_str());
                cocos2d::unzClose(zipfile);
                return false;
            }
            
            // Create a file to store current file.
            FILE *out = fopen(fullPath.c_str(), "wb");
            if (!out)
            {
                CCLOG("FileUtilEx::unzip2Folder: can not create decompress destination file %s\n", fullPath.c_str());
                cocos2d::unzCloseCurrentFile(zipfile);
                cocos2d::unzClose(zipfile);
                return false;
            }
            
            // Write current file content to destinate file.
            int error = UNZ_OK;
            do
            {
                error = cocos2d::unzReadCurrentFile(zipfile, readBuffer, BUFFER_SIZE);
                if (error < 0)
                {
                    CCLOG("FileUtilEx::unzip2Folder: can not read zip file %s, error code is %d\n", fileName, error);
                    fclose(out);
                    cocos2d::unzCloseCurrentFile(zipfile);
                    cocos2d::unzClose(zipfile);
                    return false;
                }
                
                if (error > 0)
                {
                    fwrite(readBuffer, error, 1, out);
                }
            } while(error > 0);
            
            fclose(out);
        }
        
        cocos2d::unzCloseCurrentFile(zipfile);
        
        FileUtilEx::setFileTime(fullPath, FileUtilEx::dosDateToTime(fileInfo.dosDate));
        FileUtilEx::skipiCloudBackup(fullPath);
        
        // Goto next entry listed in the zip file.
        if ((i+1) < global_info.number_entry)
        {
            if (cocos2d::unzGoToNextFile(zipfile) != UNZ_OK)
            {
                CCLOG("FileUtilEx::unzip2Folder: can not read next file for decompressing\n");
                cocos2d::unzClose(zipfile);
                return false;
            }
        }
    }
    
    cocos2d::unzClose(zipfile);
    
    cocos2d::FileUtils::getInstance()->removeFile(folderPath + fileName(fzipPath));
    
    return false;
}

#endif

#endif