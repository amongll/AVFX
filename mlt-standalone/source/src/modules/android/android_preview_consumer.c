/*
 * android_preview_consumer.c
 *
 *  Created on: 2016Äê1ÔÂ22ÈÕ
 *      Author: li.lei
 */

#include "android_preview_consumer.h"

#define __MAGIC_LOCAL__ 0xace8761d

typedef struct consumer_local_S {
	uint32_t _magic;
	pthread_mutex_t lock;
	ANativeWindow *vout;
} consumer_local_t;


int android_preview_consumer_vout_created(mlt_consumer obj, JNIEnv* env, jobject out)
{
	if (obj == NULL || out == NULL)
		return -1;
	consumer_local_t *local_obj = (consumer_local_t*)obj->local;
	if ( local_obj == NULL || local_obj->_magic != __MAGIC_LOCAL__ ) {
		return -1;
	}

	pthread_mutex_lock(&local_obj->lock);
	ANativeWindow* _tmp_out = ANativeWindow_fromSurface(env, out);
	if (_tmp_out == NULL) {
		pthread_mutex_unlock(&local_obj->lock);
		return -1;
	}
	if (local_obj->vout) {
		ANativeWindow_release(local_obj->vout);
	}
	ANativeWindow_acquire(_tmp_out);
	local_obj->vout = _tmp_out;
	pthread_mutex_unlock(&local_obj->lock);
	return 0;
}

int android_preview_consumer_vout_destroyed(mlt_consumer obj)
{
	if (obj == NULL)
		return -1;
	consumer_local_t *local_obj = (consumer_local_t*)obj->local;
	if ( local_obj == NULL || local_obj->_magic != __MAGIC_LOCAL__ ) {
		return -1;
	}
	pthread_mutex_lock(&local_obj->lock);
	if (local_obj->vout)
		ANativeWindow_release(local_obj->vout);

	local_obj->vout = NULL;
	pthread_mutex_unlock(&local_obj->lock);
	return 0;
}
