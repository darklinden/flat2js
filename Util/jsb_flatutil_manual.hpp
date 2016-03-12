//
//  jsb_flatutil_manual.hpp
//  fish_jinchan
//
//  Created by HanShaokun on 11/3/2016.
//
//

#ifndef jsb_flatutil_manual_hpp
#define jsb_flatutil_manual_hpp

#include <stdio.h>

#include "jsapi.h"
#include "jsfriendapi.h"

extern JSClass  *jsb_FlatUtil_class;
extern JSObject *jsb_FlatUtil_prototype;

void js_register_FlatUtil(JSContext *cx, JS::HandleObject global);

// return default json
bool js_FlatUtil_default4js(JSContext *cx, uint32_t argc, jsval *vp);

// convert json to buffer
bool js_FlatUtil_js2flat(JSContext *cx, uint32_t argc, jsval *vp);

// convert buffer to json
bool js_FlatUtil_flat2js(JSContext *cx, uint32_t argc, jsval *vp);

// clear all parsers
bool js_FlatUtil_clear(JSContext *cx, uint32_t argc, jsval *vp);

#endif /* jsb_flatutil_manual_hpp */
