//
//  jsb_flatutil_manual.cpp
//  fish_jinchan
//
//  Created by HanShaokun on 11/3/2016.
//
//

#include "jsb_flatutil_manual.hpp"
#include "cocos2d_specifics.hpp"
#include "cocos2d.h"
#include "CCAsyncTaskPool.h"
#include "component/CCComponentJS.h"

#include "FlatUtil.h"

static bool js_is_native_obj(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    args.rval().setBoolean(true);
    return true;
}

static bool empty_constructor(JSContext *cx, uint32_t argc, jsval *vp) {
    return false;
}

JSClass  *jsb_FlatUtil_class;
JSObject *jsb_FlatUtil_prototype;

// return default json
bool js_FlatUtil_default4js(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    bool ok = true;
    if (2 == argc) {
        std::string arg0;
        ok &= jsval_to_std_string(cx, args.get(0), &arg0);
        JSB_PRECONDITION2(ok, cx, false, "js_FlatUtil_default4js : Error processing arguments");
        
        std::string arg1;
        ok &= jsval_to_std_string(cx, args.get(1), &arg1);
        JSB_PRECONDITION2(ok, cx, false, "js_FlatUtil_default4js : Error processing arguments");
        
        auto ret = FlatUtil::default4js(arg0, arg1);
        jsval jsret = JSVAL_NULL;
        jsret = std_string_to_jsval(cx, ret);
        args.rval().set(jsret);
        return true;
    }
    
    JS_ReportError(cx, "js_FlatUtil_default4js : wrong number of arguments");
    return false;
}

// convert json to buffer
bool js_FlatUtil_js2flat(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    bool ok = true;
    
    if (4 == argc) {
        std::string arg0;
        ok &= jsval_to_std_string(cx, args.get(0), &arg0);
        JSB_PRECONDITION2(ok, cx, false, "js_FlatUtil_js2flat : Error processing arguments");
        
        std::string arg1;
        ok &= jsval_to_std_string(cx, args.get(1), &arg1);
        JSB_PRECONDITION2(ok, cx, false, "js_FlatUtil_js2flat : Error processing arguments");
        
        std::string arg2;
        ok &= jsval_to_std_string(cx, args.get(2), &arg2);
        JSB_PRECONDITION2(ok, cx, false, "js_FlatUtil_js2flat : Error processing arguments");
        
        bool arg3;
        arg3 = JS::ToBoolean(args.get(3));
        
        auto data = FlatUtil::js2flat(arg0, arg1, arg2, arg3);
        
        JSObject* buffer = JS_NewArrayBuffer(cx, static_cast<uint32_t>(data.getSize()));
        uint8_t* bufdata = JS_GetArrayBufferData(buffer);
        memcpy((void*)bufdata, (void*)data.getBytes(), data.getSize());
        jsval jsret = JSVAL_NULL;
        jsret = OBJECT_TO_JSVAL(buffer);
        args.rval().set(jsret);
        return true;
    }
    
    JS_ReportError(cx, "js_FlatUtil_js2flat : wrong number of arguments");
    return false;
}

// convert buffer to json
bool js_FlatUtil_flat2js(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    bool ok = true;
    
    if (4 == argc) {
        std::string arg0;
        ok &= jsval_to_std_string(cx, args.get(0), &arg0);
        JSB_PRECONDITION2(ok, cx, false, "js_FlatUtil_flat2js : Error processing arguments");
        
        cocos2d::Data arg1;
        uint8_t *bufdata = NULL;
        uint32_t len = 0;
        
        JSObject* jsobj = args.get(1).toObjectOrNull();
        if (JS_IsArrayBufferObject(jsobj))
        {
            bufdata = JS_GetArrayBufferData(jsobj);
            len = JS_GetArrayBufferByteLength(jsobj);
        }
        else if (JS_IsArrayBufferViewObject(jsobj))
        {
            bufdata = (uint8_t*)JS_GetArrayBufferViewData(jsobj);
            len = JS_GetArrayBufferViewByteLength(jsobj);
        }
        
        JSB_PRECONDITION2(bufdata && len > 0, cx, false, "js_FlatUtil_flat2js : Error processing arguments");
        arg1.copy(bufdata, len);
        
        std::string arg2;
        ok &= jsval_to_std_string(cx, args.get(2), &arg2);
        JSB_PRECONDITION2(ok, cx, false, "js_FlatUtil_flat2js : Error processing arguments");
        
        bool arg3;
        arg3 = JS::ToBoolean(args.get(3));
        
        auto ret = FlatUtil::flat2js(arg0, arg1, arg2, arg3);
        
        jsval jsret = JSVAL_NULL;
        jsret = std_string_to_jsval(cx, ret);
        args.rval().set(jsret);
        return true;
    }
    
    JS_ReportError(cx, "js_FlatUtil_flat2js : wrong number of arguments");
    return false;
}

// clear all parsers
bool js_FlatUtil_clear(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    
    if (argc == 0) {
        FlatUtil::clear();
        args.rval().setUndefined();
        return true;
    }
    JS_ReportError(cx, "js_FlatUtil_clear : wrong number of arguments");
    return false;
}

extern JSObject *jsb_FlatUtil_prototype;

void js_register_FlatUtil(JSContext *cx, JS::HandleObject global) {
    jsb_FlatUtil_class = (JSClass *)calloc(1, sizeof(JSClass));
    jsb_FlatUtil_class->name = "FlatUtil";
    jsb_FlatUtil_class->addProperty = JS_PropertyStub;
    jsb_FlatUtil_class->delProperty = JS_DeletePropertyStub;
    jsb_FlatUtil_class->getProperty = JS_PropertyStub;
    jsb_FlatUtil_class->setProperty = JS_StrictPropertyStub;
    jsb_FlatUtil_class->enumerate = JS_EnumerateStub;
    jsb_FlatUtil_class->resolve = JS_ResolveStub;
    jsb_FlatUtil_class->convert = JS_ConvertStub;
    jsb_FlatUtil_class->finalize = jsb_ref_finalize;
    jsb_FlatUtil_class->flags = JSCLASS_HAS_RESERVED_SLOTS(2);
    
    static JSPropertySpec properties[] = {
        JS_PSG("__nativeObj", js_is_native_obj, JSPROP_PERMANENT | JSPROP_ENUMERATE),
        JS_PS_END
    };
    
    static JSFunctionSpec *funcs = NULL;
    
    static JSFunctionSpec st_funcs[] = {
        JS_FN("default4js", js_FlatUtil_default4js, 2, JSPROP_PERMANENT | JSPROP_ENUMERATE),
        JS_FN("flat2js", js_FlatUtil_flat2js, 4, JSPROP_PERMANENT | JSPROP_ENUMERATE),
        JS_FN("js2flat", js_FlatUtil_js2flat, 4, JSPROP_PERMANENT | JSPROP_ENUMERATE),
        JS_FN("clear", js_FlatUtil_clear, 0, JSPROP_PERMANENT | JSPROP_ENUMERATE),
        JS_FS_END
    };
    
    jsb_FlatUtil_prototype = JS_InitClass(
                                          cx, global,
                                          JS::NullPtr(),
                                          jsb_FlatUtil_class,
                                          empty_constructor, 0,
                                          properties,
                                          funcs,
                                          NULL, // no static properties
                                          st_funcs);
    
    // add the proto and JSClass to the type->js info hash table
    JS::RootedObject proto(cx, jsb_FlatUtil_prototype);
    jsb_register_class<FlatUtil>(cx, jsb_FlatUtil_class, proto, JS::NullPtr());
}