/*
 * MltFactoryJNI.c
 *
 *  Created on: 2016Äê1ÔÂ15ÈÕ
 *      Author: li.lei@youku.com
 */

#include "MltJNI.h"

#include <pthread.h>
#include <framework/mlt.h>
#include <android/log.h>
#include <framework/mlt_log.h>
#include <framework/mlt_android_env.h>

typedef struct YkopMltHookInfo {
	jclass clazz;
} YkopMltHookInfo;

static YkopMltHookInfo gHook;

static jboolean FactoryInit(JNIEnv* env,jclass clazz, jobjectArray plugins, jstring jlogTag, jint jlogLevel,
		jstring filesRoot, jobject assetMgr);
static void FactoryClose(JNIEnv* env, jobject thiz);

static JNINativeMethod gMethods[] = {
		{"init", "([Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;Landroid/content/res/AssetManager;)Z", (void*)FactoryInit},
		{"close", "()V", (void*)FactoryClose}
};

#define YKOP_MLT_CLASS "com/youku/cloud/vedit/MltFactory"

bool YkopMltFactoryBind(JNIEnv* env)
{
	FIND_JAVA_CLASS(env, gHook.clazz,  YKOP_MLT_CLASS);
	if ( (*env)->RegisterNatives(env, gHook.clazz, gMethods,
		sizeof(gMethods)/sizeof(JNINativeMethod)) < 0 )
		return false;
	return true;
}

static pthread_mutex_t g_repository_lock = PTHREAD_MUTEX_INITIALIZER;
static mlt_repository g_repository = NULL;

static int mltLogLevel = MLT_LOG_DEBUG;
static char mltLogTag[128] = "mlt_debug";

static void MltLogCallback( void* ptr, int level, const char* fmt, va_list vl )
{
	int print_prefix = 1;
	mlt_properties properties = ptr ? MLT_SERVICE_PROPERTIES( ( mlt_service )ptr ) : NULL;
	char prefixbuf[512] = "";
	char logbuf[1024] = "";

	static const char* prefix_fmt1= "[%s %s] %s\n";
	static const char* prefix_fmt2= "[%s %p] %s\n";

	if ( level > mltLogLevel ) {
		return;
	}
	if ( print_prefix && properties )
	{
		char *mlt_type = mlt_properties_get( properties, "mlt_type" );
		char *mlt_service = mlt_properties_get( properties, "mlt_service" );
		char *resource = mlt_properties_get( properties, "resource" );

		if ( !( resource && *resource && resource[0] == '<' && resource[ strlen(resource) - 1 ] == '>' ) )
			mlt_type = mlt_properties_get( properties, "mlt_type" );
		if (mlt_service) {
			snprintf(prefixbuf, sizeof(prefixbuf), prefix_fmt1, mlt_type, mlt_service, resource);
		}
		else {
			snprintf(prefixbuf, sizeof(prefixbuf), prefix_fmt2, mlt_type, ptr, resource);
		}
	}
	print_prefix = strstr( fmt, "\n" ) != NULL;
	vsnprintf( logbuf,sizeof(logbuf), fmt, vl );

	if (print_prefix) {
		__android_log_print(ANDROID_LOG_ERROR, mltLogTag, "%s%s", prefixbuf, logbuf);
	}
	else {
		__android_log_print(ANDROID_LOG_ERROR, mltLogTag, "%s", logbuf);
	}
}

static jboolean FactoryInit(JNIEnv* env,jclass clazz, jobjectArray plugins, jstring jlogTag, jint jlogLevel,
		jstring jfilesRoot, jobject assetMgr)
{
	pthread_mutex_lock(&g_repository_lock);
	if ( g_repository ) {
		pthread_mutex_unlock(&g_repository_lock);
		return true;
	}

	{
		const char* logTag = (*env)->GetStringUTFChars(env, jlogTag, NULL);
		snprintf(mltLogTag, sizeof(mltLogTag), "%s", logTag);

		mltLogLevel = MLT_LOG_INFO;
		switch (jlogLevel) {
		case ANDROID_LOG_VERBOSE:
			mltLogLevel = MLT_LOG_VERBOSE;
			break;
		case ANDROID_LOG_DEBUG:
			mltLogLevel = MLT_LOG_DEBUG;
			break;
		case ANDROID_LOG_INFO:
			mltLogLevel = MLT_LOG_INFO;
			break;
		case ANDROID_LOG_WARN:
			mltLogLevel = MLT_LOG_WARNING;
			break;
		case ANDROID_LOG_ERROR:
			mltLogLevel = MLT_LOG_ERROR;
			break;
		case ANDROID_LOG_FATAL:
			mltLogLevel = MLT_LOG_FATAL;
			break;
		}

		mlt_log_set_callback(MltLogCallback);
		mlt_log_set_level(mltLogLevel);
	}

	{
		AAssetManager* mgr = AAssetManager_fromJava(env, assetMgr);
		const char* filesRoot = (*env)->GetStringUTFChars(env,jfilesRoot, NULL);
		char err[1024];
		mlt_android_check_data(mgr, filesRoot, err, sizeof(err));
	}

	int count = 0, ess_count = 0, i = 0;
	if ( plugins == NULL ) {
		return JNI_FALSE;
	}

	count = (*env)->GetArrayLength(env, plugins), ess_count = 0;

	if ( count == 0 ) {
		return JNI_FALSE;
	}


	for ( i = 0; i < count; i++ ) {
		jstring ck_plugin = (*env)->GetObjectArrayElement(env, plugins, i);
		if ( ck_plugin ) ess_count++;
	}

	if (ess_count == 0 ) {
		return JNI_FALSE;
	}

	{
		const char** plugin_names = (const char**)malloc(ess_count * sizeof(char*));
		const char** pp = plugin_names;
		for ( i=0; i<count; i++ )  {
			jstring ck_plugin = (*env)->GetObjectArrayElement(env, plugins, i);
			if ( !ck_plugin )
				continue;

			*pp = (*env)->GetStringUTFChars(env, ck_plugin, NULL);
			pp++;
		}

		g_repository = mlt_factory_init2(plugin_names, ess_count);
		free(plugin_names);
	}

	pthread_mutex_unlock(&g_repository_lock);
	return JNI_TRUE;
}

static void FactoryClose(JNIEnv* env, jclass clazz)
{
	pthread_mutex_lock(&g_repository_lock);
	if ( g_repository ) {
		mlt_factory_close2();
		g_repository = NULL;
	}
	pthread_mutex_unlock(&g_repository_lock);
}

