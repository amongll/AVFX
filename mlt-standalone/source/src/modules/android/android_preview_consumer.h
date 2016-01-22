/*
 * android_preview_consumer.h
 *
 *  Created on: 2016��1��22��
 *      Author: li.lei
 */

#ifndef ANDROID_PREVIEW_CONSUMER_H_
#define ANDROID_PREVIEW_CONSUMER_H_

#ifdef __ANDROID__
#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "framework/mlt_consumer.h"

int android_preview_consumer_vout_created(mlt_consumer obj, jobject out);
int android_preview_consumer_vout_destroyed(mlt_consumer obj);

int android_preview_consumer_aout_created(mlt_consumer obj);
int android_preview_consumer_aout_destroyed(mlt_consumer obj);

#endif

#endif /* ANDROID_PREVIEW_CONSUMER_H_ */
