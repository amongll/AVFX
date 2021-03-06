/*
 * mlt_android_env.h
 *
 *  Created on: 2016��1��18��
 *      Author: L-F000000-PC
 */

#ifndef FRAMEWORK_MLT_ANDROID_ENV_H_
#define FRAMEWORK_MLT_ANDROID_ENV_H_

#ifdef __ANDROID__
#include <jni.h>
#include <android/asset_manager_jni.h>
#include <android/native_window.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "mlt_properties.h"

mlt_properties mlt_properties_load_AAsset(AAssetManager* mgr, const char *assetName );
int mlt_properties_preset_AAsset(AAssetManager* mgr, mlt_properties self, const char *assetName );
bool mlt_android_quick_copy_AAssets(AAssetManager* mgr, const char* files_path, char *err, size_t err_size);

bool mlt_android_env_init(JavaVM* vm);
void mlt_android_env_destroy();
JNIEnv* mlt_android_get_jnienv();
void mlt_android_jnienv_thrclean();
int mlt_android_sdk_version();

#ifdef DEBUG
extern ANativeWindow* g_testAWindow;
#endif

/**
extern bool mlt_android_surface_regist(ANativeWindow* surface, const char* id);
extern bool mlt_android_surface_detach(ANativeWindow* surface, const char* id);
extern ANativeWindow* mlt_android_surface_get(const char* id);
**/

#endif

#endif /* FRAMEWORK_MLT_ANDROID_ENV_H_ */
