/*
 * mlt_android_jni_util.h
 *
 *  Created on: 2016��2��15��
 *      Author: li.lei
 */

#ifndef MLT_ANDROID_JNI_UTIL_H_
#define MLT_ANDROID_JNI_UTIL_H_

#ifdef __ANDROID__
#include <jni.h>
#include <android/log.h>
static inline bool JNI_CatchJVMException(JNIEnv* env) {
#ifndef __cplusplus
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return true;
    }
#else
    if ( env->ExceptionCheck()) {
    	env->ExceptionDescribe();
    	env->ExceptionClear();
    	return true;
    }
#endif

    return false;
}

static inline bool JNI_RethrowJVMException(JNIEnv *env)
{
#ifndef __cplusplus
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionDescribe(env);
        return true;
    }
#else
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        return true;
    }
#endif

    return false;
}

extern const char* LOGTAG;


#define ALOGV(fmt, ...)  __android_log_print(ANDROID_LOG_VERBOSE,   LOGTAG,  (fmt), ##__VA_ARGS__)
#define ALOGD(fmt, ...)  __android_log_print(ANDROID_LOG_DEBUG,     LOGTAG,  (fmt), ##__VA_ARGS__)
#define ALOGI(fmt, ...)  __android_log_print(ANDROID_LOG_INFO,      LOGTAG,  (fmt), ##__VA_ARGS__)
#define ALOGW(fmt, ...)  __android_log_print(ANDROID_LOG_WARN,      LOGTAG,  (fmt), ##__VA_ARGS__)
#define ALOGE(fmt, ...)  __android_log_print(ANDROID_LOG_ERROR,     LOGTAG,  (fmt), ##__VA_ARGS__)

#ifndef __cplusplus
#define FIND_JAVA_CLASS(env__, var__, classsign__, eret__) \
do { \
	jclass clazz = (*env__)->FindClass(env__, classsign__); \
	if (JNI_CatchJVMException(env__) || !(clazz)) { \
		ALOGE("FindClass failed: %s", classsign__); \
		return (eret__); \
	} \
	var__ = (*env__)->NewGlobalRef(env__, clazz); \
	if (JNI_CatchJVMException(env__) || !(var__)) { \
		(*env__)->DeleteLocalRef(env__, clazz); \
		return (eret__); \
	} \
	(*env__)->DeleteLocalRef(env__, clazz); \
} while(0);


#define FIND_JAVA_METHOD(env__, var__, clazz__, name__, sign__, eret__) \
do { \
	(var__) = (*env__)->GetMethodID((env__), (clazz__), (name__), (sign__)); \
	if (JNI_CatchJVMException(env__) || !(var__)) { \
		ALOGE("GetMethodID failed: %s", name__); \
		return (eret__); \
	} \
} while(0);

#define FIND_JAVA_FIELD(env__, var__, clazz__, name__, sign__ , eret__) \
do {\
	(var__) = (*env__)->GetFieldID((env__),(clazz__), (name__), (sign__));\
	if (JNI_CatchJVMException(env__) || !(var__)) { \
		ALOGE("GetFieldID failed: %s", name__); \
		return (eret__); \
	} \
}while(0);

#define FIND_JAVA_STATIC_METHOD(env__, var__, clazz__, name__, sign__, eret__) \
    do { \
        (var__) = (*env__)->GetStaticMethodID((env__), (clazz__), (name__), (sign__)); \
        if (JNI_CatchJVMException(env__) || !(var__)) { \
            ALOGE("GetStaticMethodID failed: %s", name__); \
            return (eret__); \
        } \
    } while(0);


#define FIND_JAVA_STATIC_FIELD(env__, var__, clazz__, name__, sign__, eret__) \
    do { \
        (var__) = (*env__)->GetStaticFieldID((env__), (clazz__), (name__), (sign__)); \
        if (JNI_CatchJVMException(env__) || !(var__)) { \
            ALOGE("GetStaticFieldID failed: %s", name__); \
            return (eret__); \
        } \
    } while(0);
#else
#define FIND_JAVA_CLASS(env__, var__, classsign__, eret__) \
do { \
	jclass clazz = (jclass)(env__)->FindClass(classsign__); \
	if (JNI_CatchJVMException((env__)) || !(clazz)) { \
		__android_log_print(ANDROID_LOG_ERROR, LOGTAG, "FindClass failed: %s", classsign__);\
		return (eret__); \
	} \
	var__ = (jclass)(env__)->NewGlobalRef(clazz); \
	if (JNI_CatchJVMException((env__)) || !(var__)) { \
		(env__)->DeleteLocalRef(clazz); \
		return (eret__); \
	} \
	(env__)->DeleteLocalRef(clazz); \
} while(0);


#define FIND_JAVA_METHOD(env__, var__, clazz__, name__, sign__, eret__) \
do { \
	(var__) = (env__)->GetMethodID((clazz__), (name__), (sign__)); \
	if (JNI_CatchJVMException((env__)) || !(var__)) { \
		__android_log_print(ANDROID_LOG_ERROR, LOGTAG, "GetMethodID failed: %s", name__); \
		return (eret__); \
	} \
} while(0);

#define FIND_JAVA_FIELD(env__, var__, clazz__, name__, sign__ , eret__) \
do {\
	(var__) = (env__)->GetFieldID((clazz__), (name__), (sign__));\
	if (JNI_CatchJVMException((env__)) || !(var__)) { \
		__android_log_print(ANDROID_LOG_ERROR, LOGTAG, "GetFieldID failed: %s", name__); \
		return (eret__); \
	} \
}while(0);

#define FIND_JAVA_STATIC_METHOD(env__, var__, clazz__, name__, sign__, eret__) \
    do { \
        (var__) = (env__)->GetStaticMethodID((clazz__), (name__), (sign__)); \
        if (JNI_CatchJVMException((env__)) || !(var__)) { \
            __android_log_print(ANDROID_LOG_ERROR, LOGTAG, "GetStaticMethodID failed: %s", name__); \
            return (eret__); \
        } \
    } while(0);


#define FIND_JAVA_STATIC_FIELD(env__, var__, clazz__, name__, sign__, eret__) \
    do { \
        (var__) = (env__)->GetStaticFieldID((clazz__), (name__), (sign__)); \
        if (JNI_CatchJVMException((env__)) || !(var__)) { \
        	__android_log_print(ANDROID_LOG_ERROR, LOGTAG, "GetStaticFieldID failed: %s", name__); \
            return (eret__); \
        } \
    } while(0);
#endif


#endif
#endif /* SRC_FRAMEWORK_MLT_ANDROID_JNI_UTIL_H_ */
