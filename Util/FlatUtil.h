//
//  FlatUtil.hpp
//  fish_jinchan
//
//  Created by HanShaokun on 10/3/2016.
//
//

#ifndef FlatUtil_hpp
#define FlatUtil_hpp

#include "cocos2d.h"
#include "external/flatbuffers/idl.h"
#include "external/flatbuffers/util.h"
#include "external/flatbuffers/flatbuffers.h"
#include <map>

class FlatUtil {
    
public:
    // return default json
    static std::string default4js(const std::string& fbs_file,
                                  const std::string& fbs_name = "");
    
    // convert json to buffer
    static cocos2d::Data js2flat(const std::string& fbs_file,
                                 const std::string& json,
                                 const std::string& fbs_name = "",
                                 bool cache_the_parser = false);
    
    // convert buffer to json
    static std::string flat2js(const std::string& fbs_file,
                               const cocos2d::Data& data,
                               const std::string& fbs_name = "",
                               bool cache_the_parser = false);
    
    // clear all parsers
    static void clear();
    
private:
    
    std::map<std::string, flatbuffers::Parser*> _stored_parsers;
    
    const char* _include_directories[3];
    
    static FlatUtil* getInstance();
    
    void _clear();
    
    std::string _default4js(const std::string& fbs_file,
                            const std::string& fbs_name);
    
    cocos2d::Data _js2flat(const std::string& fbs_file,
                           const std::string& json,
                           const std::string& fbs_name,
                           bool cache_the_parser);
    
    std::string _flat2js(const std::string& fbs_file,
                         const cocos2d::Data& data,
                         const std::string& fbs_name,
                         bool cache_the_parser);
    
};

#endif /* FlatUtil_hpp */
