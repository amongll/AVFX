/*
 * VEditAndroid.cpp
 *
 *  Created on: 2016年3月29日
 *      Author: li.lei@youku.com
 */

#ifdef __ANDROID__

#include "framework/mlt.h"
#include <jni.h>

namespace vedit {
	bool VEditVMJniLoad(JNIEnv* jnienv);
	bool VEditMltRuntimeJniLoad(JNIEnv* jnienv);
}

#ifdef __cplusplus
extern "C" {
#endif


JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env = NULL;
	if ( vm->GetEnv((void**)&env,JNI_VERSION_1_4) != JNI_OK )
		return -1;

	if ( !mlt_android_env_init(vm) ) return -1;
	if ( !vedit::VEditVMJniLoad(env)) return -1;
	if ( !vedit::VEditMltRuntimeJniLoad(env) ) return -1;

	return JNI_VERSION_1_4;
}

JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved)
{
	mlt_android_env_destroy();
	return;
}

#ifdef __cplusplus
}
#endif

#endif


