
#pragma once

#include <android/log.h>

#define LOG_TAG "Reedle"

#define LOG(level, ...) __android_log_print(level, LOG_TAG, __VA_ARGS__)
#define LOGV(...) LOG(ANDROID_LOG_VERBOSE, __VA_ARGS__)
#define LOGD(...) LOG(ANDROID_LOG_DEBUG, __VA_ARGS__)
#define LOGI(...) LOG(ANDROID_LOG_INFO, __VA_ARGS__)
#define LOGW(...) LOG(ANDROID_LOG_WARN, __VA_ARGS__)
#define LOGE(...) LOG(ANDROID_LOG_ERROR, __VA_ARGS__)
#define LOGF(...) LOG(ANDROID_LOG_FATAL, __VA_ARGS__)
