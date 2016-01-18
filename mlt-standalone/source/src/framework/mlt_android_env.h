/*
 * mlt_android_env.h
 *
 *  Created on: 2016Äê1ÔÂ18ÈÕ
 *      Author: L-F000000-PC
 */

#ifndef FRAMEWORK_MLT_ANDROID_ENV_H_
#define FRAMEWORK_MLT_ANDROID_ENV_H_

#ifdef ANDROID
#include <jni.h>
#include <android/asset_manager_jni.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "mlt_properties.h"
extern JavaVM* gJavaVm;

extern mlt_properties mlt_properties_load_AAsset(AAssetManager* mgr, const char *assetName );
extern int mlt_properties_preset_AAsset(AAssetManager* mgr, mlt_properties self, const char *assetName );
extern bool mlt_android_check_data(AAssetManager* mgr, const char* files_path, char *err, size_t err_size);
#endif

#endif /* FRAMEWORK_MLT_ANDROID_ENV_H_ */
