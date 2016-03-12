//
//  FileUtilEx.cpp
//
//  Created by darklinden on 11/7/14.
//
//

#include "FileUtilEx.h"

#if CC_TARGET_PLATFORM != CC_PLATFORM_IOS

#if ENABLE_ZIP
#define BUFFER_SIZE    8192
#define MAX_FILENAME   512
#include "unzip/unzip.h"

#if (CC_PLATFORM_ANDROID == CC_TARGET_PLATFORM)
#include <jni.h>
#include "platform/android/jni/JniHelper.h"
#include <android/log.h>
#endif
#endif

#ifndef WIN32
#include <unistd.h>
#include <dirent.h>
#else
#include <Windows.h>
#include "Shlobj.h"
#include "Shldisp.h"
#include <atlcomcli.h>
/** This file is part of the Mingw32 package. * unistd.h maps (roughly) to io.h */
#ifndef _UNISTD_H
#define _UNISTD_H
#include <io.h>
#include <process.h>
#endif /* _UNISTD_H */
#endif


#ifdef WIN32
int SearchDirectory(std::vector<std::string> &refvecFiles,
                    const std::string        &refcstrRootDirectory,
                    const std::string        &refcstrExtension,
                    bool                     bSearchSubdirectories = true)
{
    std::string     strFilePath;             // Filepath
    std::string     strPattern;              // Pattern
    std::string     strExtension;            // Extension
    HANDLE          hFile;                   // Handle to file
    WIN32_FIND_DATAA FileInformation;         // File information
    
    
    strPattern = refcstrRootDirectory + "\\*.*";
    
    hFile = ::FindFirstFileA(strPattern.c_str(), &FileInformation);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (FileInformation.cFileName[0] != '.')
            {
                strFilePath.erase();
                strFilePath = refcstrRootDirectory + "\\" + FileInformation.cFileName;
                
                if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (bSearchSubdirectories)
                    {
                        // Search subdirectory
                        int iRC = SearchDirectory(refvecFiles,
                                                  strFilePath,
                                                  refcstrExtension,
                                                  bSearchSubdirectories);
                        if (iRC)
                            return iRC;
                    }
                }
                else
                {
                    // Check extension
                    strExtension = FileInformation.cFileName;
                    strExtension = strExtension.substr(strExtension.rfind(".") + 1);
                    
                    if (refcstrExtension.length())
                    {
                        // Save filename
                        if (strExtension == refcstrExtension)
                        {
                            refvecFiles.push_back(strFilePath);
                        }
                    }
                    else
                    {
                        refvecFiles.push_back(strFilePath);
                    }
                }
            }
        } while (::FindNextFileA(hFile, &FileInformation) == TRUE);
        
        // Close handle
        ::FindClose(hFile);
        
        DWORD dwError = ::GetLastError();
        if (dwError != ERROR_NO_MORE_FILES)
            return dwError;
    }
    
    return 0;
}
#endif

std::vector<std::string> FileUtilEx::contentOfFolder(const std::string& path)
{
#ifndef WIN32
    std::vector<std::string> ret;
    ret.clear();
    
    if (!cocos2d::FileUtils::getInstance()->isDirectoryExist(path)) {
        return ret;
    }
    
    DIR*            dp;
    struct dirent*  ep;
    
    dp = opendir(path.c_str());
    
    while ((ep = readdir(dp)) != NULL) {
        if (std::string(ep->d_name) == "." || std::string(ep->d_name) == "..") continue;
        ret.push_back(std::string(ep->d_name));
    }
    
    closedir(dp);
    return ret;
#else
    
    int                      iRC = 0;
    std::vector<std::string> vecFiles;
    
    // Search 'c:' for '.avi' files including subdirectories
    iRC = SearchDirectory(vecFiles, path, "");
    if (iRC)
    {
        vecFiles.clear();
    }
    
    return vecFiles;
    
#endif
}

std::string FileUtilEx::resPath()
{
#ifndef WIN32
    return "";
#else
    std::string ret = "";
    char ownPth[MAX_PATH];
    
    // Will contain exe path
    HMODULE hModule = GetModuleHandleA(NULL);
    if (hModule != NULL)
    {
        // When passing NULL to GetModuleHandle, it returns handle of exe itself
        GetModuleFileNameA(hModule, ownPth, (sizeof(ownPth)));
        //CCLOG("AppDelegate::AppDelegate GetModuleFileNameA %s", ownPth);
        std::string path = ownPth;
        ret = path;
        
        auto pos = path.rfind("\\");
        if (pos != path.npos)
        {
            path = path.substr(0, pos);
            ret = path;
        }
    }
    
    return ret;
#endif
}

void FileUtilEx::skipiCloudBackup(const std::string& path)
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
    
#if (CC_PLATFORM_ANDROID == CC_TARGET_PLATFORM)
    fzipPath = desPath + fileName(zip);
    
    // read asset data
    AAsset* asset =
    AAssetManager_open(FileUtilsAndroid::assetmanager,
                       relativePath.c_str(),
                       AASSET_MODE_UNKNOWN);
    if (nullptr == asset) {
        LOGD("asset is nullptr");
        return Data::Null;
    }
    
    off_t fileSize = AAsset_getLength(asset);
    
    if (forString)
    {
        data = (unsigned char*) malloc(fileSize + 1);
        data[fileSize] = '\0';
    }
    else
    {
        data = (unsigned char*) malloc(fileSize);
    }
    
    int bytesread = AAsset_read(asset, (void*)data, fileSize);
    size = bytesread;
    
    AAsset_close(asset);
#endif
    
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
        CCLOG("FileUtilEx::unzip2Folder: extract file %s\n", fullPath.c_str());
        
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

/*
 // FUNCTIONS FOR WIN32
 
 std::string FileUtilEx::getInstallPath()
 {
 const TCHAR* des_folder = _T("JCBY");
 
 TCHAR szPath[MAX_PATH];
 
 if (SUCCEEDED(SHGetFolderPath(NULL,
 CSIDL_PROGRAM_FILESX86 | CSIDL_FLAG_CREATE,
 NULL,
 0,
 szPath)))
 {
 auto wInstallPath = std::wstring(szPath) + _T("\\") + des_folder + _T("\\");
 std::string installPath(wInstallPath.length(), ' ');
 std::copy(wInstallPath.begin(), wInstallPath.end(), installPath.begin());
 return installPath;
 }
 else {
 if (SUCCEEDED(SHGetFolderPath(NULL,
 CSIDL_PROGRAM_FILES | CSIDL_FLAG_CREATE,
 NULL,
 0,
 szPath)))
 {
 auto wInstallPath = std::wstring(szPath) + _T("\\") + des_folder + _T("\\");
 std::string installPath(wInstallPath.length(), ' ');
 std::copy(wInstallPath.begin(), wInstallPath.end(), installPath.begin());
 return installPath;
 }
 }
 
 return "";
 }
 
 bool FileUtilEx::unzip2Folder(const std::string& zip, const std::string& folder)
 {
 IShellDispatch *pISD;
 
 Folder  *pZippedFile = 0L;
 Folder  *pDestination = 0L;
 
 long FilesCount = 0;
 IDispatch* pItem = 0L;
 FolderItems *pFilesInside = 0L;
 
 VARIANT Options, OutFolder, InZipFile, Item;
 CoInitialize(NULL);
 
 if (CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void **)&pISD) != S_OK) {
 CoUninitialize();
 return 1;
 }
 
 InZipFile.vt = VT_BSTR;
 CComBSTR wzip(zip.c_str());
 
 InZipFile.bstrVal = wzip;
 pISD->NameSpace(InZipFile, &pZippedFile);
 if (!pZippedFile)
 {
 pISD->Release();
 CoUninitialize();
 return 1;
 }
 
 OutFolder.vt = VT_BSTR;
 CComBSTR wfolder(folder.c_str());
 OutFolder.bstrVal = wfolder;
 pISD->NameSpace(OutFolder, &pDestination);
 if (!pDestination)
 {
 pZippedFile->Release();
 pISD->Release();
 CoUninitialize();
 return 1;
 }
 
 pZippedFile->Items(&pFilesInside);
 if (!pFilesInside)
 {
 pDestination->Release();
 pZippedFile->Release();
 pISD->Release();
 CoUninitialize();
 return 1;
 }
 
 pFilesInside->get_Count(&FilesCount);
 if (FilesCount < 1)
 {
 pFilesInside->Release();
 pDestination->Release();
 pZippedFile->Release();
 pISD->Release();
 CoUninitialize();
 return 0;
 }
 
 pFilesInside->QueryInterface(IID_IDispatch, (void**)&pItem);
 
 Item.vt = VT_DISPATCH;
 Item.pdispVal = pItem;
 
 Options.vt = VT_I4;
 Options.lVal = 1024 | 512 | 16 | 4;//http://msdn.microsoft.com/en-us/library/bb787866(VS.85).aspx
 
 bool retval = pDestination->CopyHere(Item, Options) == S_OK;
 
 pItem->Release(); pItem = 0L;
 pFilesInside->Release(); pFilesInside = 0L;
 pDestination->Release(); pDestination = 0L;
 pZippedFile->Release(); pZippedFile = 0L;
 pISD->Release(); pISD = 0L;
 
 CoUninitialize();
 return retval;
 }
 */