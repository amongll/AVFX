/*
 * MltJNI.h
 *
 *  Created on: 2016Äê1ÔÂ15ÈÕ
 *      Author: li.lei@youku.com
 */

#ifndef MLTJNI_H_
#define MLTJNI_H_

#ifndef ANDROID
#define ANDROID
#endif

#include <jni.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <framework/mlt.h>

extern bool YkopMltFactoryBind(JNIEnv* env);

static inline bool JNI_CatchJVMException(JNIEnv* env) {
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return true;
    }

    return false;
}

#define FIND_JAVA_CLASS(env__, var__, classsign__) \
do { \
	jclass clazz = (*env__)->FindClass(env__, classsign__); \
	if (JNI_CatchJVMException(env) || !(clazz)) { \
		return false; \
	} \
	var__ = (*env__)->NewGlobalRef(env__, clazz); \
	if (JNI_CatchJVMException(env) || !(var__)) { \
		(*env__)->DeleteLocalRef(env__, clazz); \
		return false; \
	} \
	(*env__)->DeleteLocalRef(env__, clazz); \
} while(0);


#define FIND_JAVA_METHOD(env__, var__, clazz__, name__, sign__) \
do { \
	(var__) = (*env__)->GetMethodID((env__), (clazz__), (name__), (sign__)); \
	if (JNI_CatchJVMException(env) || !(var__)) { \
		return false; \
	} \
} while(0);

#define FIND_JAVA_FIELD(env__, var__, clazz__, name__, sign__ ) \
do {\
	(var__) = (*env__)->GetFieldID((env__),(clazz__), (name__), (sign__));\
	if (JNI_CatchJVMException(env) || !(var__)) { \
		return false; \
	} \
}while(0);

extern JavaVM *gVm;

#endif /* MLTJNI_H_ */
