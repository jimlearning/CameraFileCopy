// iOS平台兼容性空头文件
// 这个文件是为了解决iOS平台上缺少JNI接口的问题
// 在iOS中，JNI是不存在的，我们需要通过其他方式实现平台交互

#pragma once

// 定义JNI空结构和类型，以便编译通过
#ifdef __cplusplus
extern "C" {
#endif

// 基本JNI类型
typedef int jint;
typedef long long jlong;
typedef void* jobject;
typedef char* jstring;

// JNIEnv结构
typedef struct JNINativeInterface JNINativeInterface;
typedef const JNINativeInterface* JNIEnv;

#ifdef __cplusplus
}  // extern "C"
#endif

// 注意：这个头文件仅提供最小程度的类型定义，使编译通过
// 在实际iOS应用中，需要使用Objective-C/Swift与C++交互，而不是JNI
