// iOS平台兼容性空头文件
// 这个文件是为了解决iOS平台上缺少Android日志API的问题

#pragma once

// 模拟Android日志优先级
enum android_LogPriority {
    ANDROID_LOG_UNKNOWN = 0,
    ANDROID_LOG_DEFAULT,
    ANDROID_LOG_VERBOSE,
    ANDROID_LOG_DEBUG,
    ANDROID_LOG_INFO,
    ANDROID_LOG_WARN,
    ANDROID_LOG_ERROR,
    ANDROID_LOG_FATAL,
    ANDROID_LOG_SILENT
};

// 提供空实现的日志函数
#define __android_log_print(prio, tag, fmt, ...) ((void)0)
