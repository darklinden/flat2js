//
//  FlatUtil.cpp
//  fish_jinchan
//
//  Created by HanShaokun on 10/3/2016.
//
//

#include "FlatUtil.h"
#include "FileUtilEx.h"

#define MYASSERT(e, msg) \
{ \
    if (!e) { cocos2d::log("ASSERT FAILURE:\n %s\n\n", std::string(msg).c_str()); } \
    assert(e); \
}

#define FLAT_FILE_PATH std::string("res/fbs/")

std::string FlatUtil::default4js(const std::string &fbs_file,
                                 const std::string &fbs_name/* = ""*/)
{
    auto name_ = fbs_name;
    if (!name_.length()) {
        name_ = fbs_file;
    }
    return FlatUtil::getInstance()->_default4js(fbs_file, name_);
}

// convert json to buffer
cocos2d::Data FlatUtil::js2flat(const std::string &fbs_file,
                                const std::string &json,
                                const std::string &fbs_name/* = ""*/,
                                bool cache_the_parser/* = false*/)
{
    auto name_ = fbs_name;
    if (!name_.length()) {
        name_ = fbs_file;
    }
    return FlatUtil::getInstance()->_js2flat(fbs_file,
                                             json,
                                             name_,
                                             cache_the_parser);
}

// convert buffer to json
std::string FlatUtil::flat2js(const std::string& fbs_file,
                              const cocos2d::Data& data,
                              const std::string& fbs_name/* = ""*/,
                              bool cache_the_parser/* = false*/)
{
    auto name_ = fbs_name;
    if (!name_.length()) {
        name_ = fbs_file;
    }
    return FlatUtil::getInstance()->_flat2js(fbs_file,
                                             data,
                                             name_,
                                             cache_the_parser);
}

// clear all parsers
void FlatUtil::clear()
{
    FlatUtil::getInstance()->_clear();
}

FlatUtil* FlatUtil::getInstance()
{
    static FlatUtil* _instanc = nullptr;
    if (!_instanc) {
        _instanc = new (std::nothrow) FlatUtil();
        _instanc->_stored_parsers.clear();
        
        auto tmp0 = cocos2d::FileUtils::getInstance()->getWritablePath() + "res/fbs/";
        char* buff0 = (char *)malloc(sizeof(char) * tmp0.length() + 1);
        memset(buff0, 0, tmp0.length() + 1);
        memcpy(buff0, tmp0.c_str(), tmp0.length());
        
        _instanc->_include_directories[0] = buff0;
        
        auto tmp = FileUtilEx::resPath() + "res/fbs/";
        char* buff = (char *)malloc(sizeof(char) * tmp.length() + 1);
        memset(buff, 0, tmp.length() + 1);
        memcpy(buff, tmp.c_str(), tmp.length());
        
        _instanc->_include_directories[1] = buff;
        
        _instanc->_include_directories[2] = nullptr;
    }
    return _instanc;
}

void FlatUtil::_clear()
{
    for (auto iter = _stored_parsers.begin(); iter != _stored_parsers.end();)
    {
       delete iter->second;
        _stored_parsers.erase(iter++);
    }
    
    _stored_parsers.clear();
}

std::string FlatUtil::_default4js(const std::string &fbs_file,
                                  const std::string &fbs_name)
{
    // create parser with fbs
    std::string schemafile;
    
    auto path = cocos2d::FileUtils::getInstance()->fullPathForFilename(FLAT_FILE_PATH + fbs_file + ".fbs");
    bool ok = flatbuffers::LoadFile(path.c_str(), false, &schemafile);
    MYASSERT(ok, std::string("FlatUtil::_flat2js get fbs [" + path + "] file failed"));
    
    schemafile = FileUtilEx::decrypt(schemafile);
    
    flatbuffers::Parser parser;
    ok = parser.Parse(schemafile.c_str(), _include_directories);
    MYASSERT(ok, std::string("FlatUtil::_flat2js load fbs [" + fbs_file + "] failed"));
    
    for (auto it = parser.structs_.vec.begin(); it != parser.structs_.vec.end(); ++it) {
        std::string jsongen = "";
        flatbuffers::StructDef &fd = **it;
        if (fd.name == fbs_name) {
            flatbuffers::GenerateTextStruct(fd, &jsongen);
            return jsongen;
        }
    }
    
    return "";
}

cocos2d::Data FlatUtil::_js2flat(const std::string &fbs_file,
                                 const std::string &json,
                                 const std::string &fbs_name,
                                 bool cache_the_parser)
{
    flatbuffers::Parser* parser = nullptr;
    if (cache_the_parser) {
        // load parser from stored
        auto parser = _stored_parsers[fbs_name];
        if (parser) {
            bool ok = parser->Parse(json.c_str(), _include_directories);
            if (ok) {
                cocos2d::Data data;
                data.copy(parser->builder_.GetBufferPointer(), parser->builder_.GetSize());
                return data;
            }
            else {
                printf("FlatUtil::_js2flat stored parser [%s] parse json [%s] failed", fbs_name.c_str(), json.c_str());
                _stored_parsers.erase(fbs_name);
                delete parser;
                parser = nullptr;
            }
        }
    }
    
    // create parser with fbs
    std::string schemafile;
    
    auto path = cocos2d::FileUtils::getInstance()->fullPathForFilename(FLAT_FILE_PATH + fbs_file + ".fbs");
    bool ok = flatbuffers::LoadFile(path.c_str(), false, &schemafile);
    MYASSERT(ok, std::string("FlatUtil::_js2flat get fbs [" + path + "] file failed"));
    
    schemafile = FileUtilEx::decrypt(schemafile);
    
    parser = new flatbuffers::Parser();
    ok = parser->Parse(schemafile.c_str(), _include_directories);
    MYASSERT(ok, std::string("FlatUtil::_js2flat load fbs [" + fbs_file + "] failed"));
    
    ok = false;
    for (auto it = parser->structs_.vec.begin(); it != parser->structs_.vec.end(); ++it) {
        std::string jsongen = "";
        flatbuffers::StructDef &fd = **it;
        if (fd.name == fbs_name) {
            ok = true;
            parser->SetRootType(fbs_name.c_str());
            if (cache_the_parser) {
                _stored_parsers[fbs_name] = parser;
            }
            break;
        }
    }
    MYASSERT(ok, std::string("FlatUtil::_js2flat create new parser [" + fbs_name + "] failed"));
    
    ok = parser->Parse(json.c_str(), _include_directories);
    MYASSERT(ok, std::string("FlatUtil::_js2flat new parser [" + fbs_name + "] parse json [" + json + "] failed"));

    cocos2d::Data data;
    data.copy(parser->builder_.GetBufferPointer(), parser->builder_.GetSize());
    return data;
}

std::string FlatUtil::_flat2js(const std::string &fbs_file,
                               const cocos2d::Data &data,
                               const std::string &fbs_name,
                               bool cache_the_parser)
{
    flatbuffers::Parser* parser = nullptr;
    if (cache_the_parser) {
        // load parser from stored
        auto parser = _stored_parsers[fbs_name];
        if (parser) {
            parser->builder_.Clear();
            
            struct flatbuffers::GeneratorOptions opt;
            std::string jsongen;
            flatbuffers::GenerateText(parser, data.getBytes(), opt, &jsongen);
            
            if (jsongen.length()) {
                return jsongen;
            }
            else {
                printf("FlatUtil::_flat2js stored parser [%s] parse buff failed", fbs_name.c_str());
                _stored_parsers.erase(fbs_name);
                delete parser;
                parser = nullptr;
            }
        }
    }

    // create parser with fbs
    std::string schemafile;
    
    auto path = cocos2d::FileUtils::getInstance()->fullPathForFilename(FLAT_FILE_PATH + fbs_file + ".fbs");
    bool ok = flatbuffers::LoadFile(path.c_str(), false, &schemafile);
    MYASSERT(ok, std::string("FlatUtil::_flat2js get fbs [" + path + "] file failed"));
    
    schemafile = FileUtilEx::decrypt(schemafile);
    
    parser = new flatbuffers::Parser();
    ok = parser->Parse(schemafile.c_str(), _include_directories);
    MYASSERT(ok, std::string("FlatUtil::_flat2js load fbs [" + fbs_file + "] failed"));
    
    ok = false;
    for (auto it = parser->structs_.vec.begin(); it != parser->structs_.vec.end(); ++it) {
        std::string jsongen = "";
        flatbuffers::StructDef &fd = **it;
        if (fd.name == fbs_name) {
            ok = true;
            parser->SetRootType(fbs_name.c_str());
            if (cache_the_parser) {
                _stored_parsers[fbs_name] = parser;
            }
            break;
        }
    }
    MYASSERT(ok, std::string("FlatUtil::_flat2js create new parser [" + fbs_name + "] failed"));
    
    struct flatbuffers::GeneratorOptions opt;
    opt.output_default_scalars_in_json = true;
    opt.lang = flatbuffers::GeneratorOptions::Language::kMAX;
    
    std::string jsongen;
    flatbuffers::GenerateText(*parser, data.getBytes(), opt, &jsongen);
    
    MYASSERT(jsongen.length(), std::string("FlatUtil::_flat2js new parser [" + fbs_name + "] parse buff failed"));
    
    return jsongen;
}

