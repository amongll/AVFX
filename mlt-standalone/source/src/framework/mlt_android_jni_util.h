/*
 * mlt_android_jni_util.h
 *
 *  Created on: 2016Äê2ÔÂ15ÈÕ
 *      Author: li.lei
 */

#ifndef MLT_ANDROID_JNI_UTIL_H_
#define MLT_ANDROID_JNI_UTIL_H_

#ifdef ANDROID
#include <jni.h>
#include <android/log.h>
static inline bool JNI_CatchJVMException(JNIEnv* env) {
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return true;
    }

    return false;
}

static inline bool JNI_RethrowJVMException(JNIEnv *env)
{
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionDescribe(env);
        return true;
    }

    return false;
}

#define VLOG(level, TAG, ...)    ((void)__android_log_vprint(level, TAG, __VA_ARGS__))
#define ALOG(level, TAG, ...)    ((void)__android_log_print(level, TAG, __VA_ARGS__))
#define LOGTAG "IJKMEDIA"


#define ALOGV(...)  ALOG(ANDROID_LOG_VERBOSE,   LOGTAG,  __VA_ARGS__)
#define ALOGD(...)  ALOG(ANDROID_LOG_DEBUG,     LOGTAG,  __VA_ARGS__)
#define ALOGI(...)  ALOG(ANDROID_LOG_INFO,      LOGTAG,  __VA_ARGS__)
#define ALOGW(...)  ALOG(ANDROID_LOG_WARN,      LOGTAG,  __VA_ARGS__)
#define ALOGE(...)  ALOG(ANDROID_LOG_ERROR,     LOGTAG,  __VA_ARGS__)


#define FIND_JAVA_CLASS(env__, var__, classsign__, eret__) \
do { \
	jclass clazz = (*env__)->FindClass(env__, classsign__); \
	if (JNI_CatchJVMException(env) || !(clazz)) { \
		ALOGE("FindClass failed: %s", classsign__); \
		return (eret__); \
	} \
	var__ = (*env__)->NewGlobalRef(env__, clazz); \
	if (JNI_CatchJVMException(env) || !(var__)) { \
		(*env__)->DeleteLocalRef(env__, clazz); \
		return (eret__); \
	} \
	(*env__)->DeleteLocalRef(env__, clazz); \
} while(0);


#define FIND_JAVA_METHOD(env__, var__, clazz__, name__, sign__, eret__) \
do { \
	(var__) = (*env__)->GetMethodID((env__), (clazz__), (name__), (sign__)); \
	if (JNI_CatchJVMException(env) || !(var__)) { \
		ALOGE("GetMethodID failed: %s", name__); \
		return (eret__); \
	} \
} while(0);

#define FIND_JAVA_FIELD(env__, var__, clazz__, name__, sign__ , eret__) \
do {\
	(var__) = (*env__)->GetFieldID((env__),(clazz__), (name__), (sign__));\
	if (JNI_CatchJVMException(env) || !(var__)) { \
		ALOGE("GetFieldID failed: %s", name__); \
		return (eret__); \
	} \
}while(0);

#define FIND_JAVA_STATIC_METHOD(env__, var__, clazz__, name__, sign__, eret__) \
    do { \
        (var__) = (*env__)->GetStaticMethodID((env__), (clazz__), (name__), (sign__)); \
        if (JNI_CatchJVMException(env) || !(var__)) { \
            ALOGE("GetStaticMethodID failed: %s", name__); \
            return (eret__); \
        } \
    } while(0);


#define FIND_JAVA_STATIC_FIELD(env__, var__, clazz__, name__, sign__, eret__) \
    do { \
        (var__) = (*env__)->GetStaticFieldID((env__), (clazz__), (name__), (sign__)); \
        if (JNI_CatchJVMException(env) || !(var__)) { \
            ALOGE("GetStaticFieldID failed: %s", name__); \
            return (eret__); \
        } \
    } while(0);


#endif
#endif /* SRC_FRAMEWORK_MLT_ANDROID_JNI_UTIL_H_ */
