/*
 * MltJNI.c
 *
 *  Created on: 2016Äê1ÔÂ15ÈÕ
 *      Author: li.lei@youku.com
 */

#include "MltJNI.h"

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env = NULL;
	if ( (*vm)->GetEnv(vm, (void**)&env,JNI_VERSION_1_4) != JNI_OK )
		return -1;

	if ( !YkopMltFactoryBind(env) ) return -1;

	mlt_android_global_javavm = vm;
	return JNI_VERSION_1_4;
}

JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved)
{
	mlt_android_global_javavm = NULL;
	return;
}
