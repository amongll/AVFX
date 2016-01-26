/*
 * android_preview_consumer.c
 *
 *  Created on: 2016年1月22日
 *      Author: li.lei
 */

#include "android_preview_consumer.h"
#include "framework/mlt_deque.h"
#include "framework/mlt_consumer.h"
#include "framework/mlt_log.h"
#include "framework/mlt_frame.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define __MAGIC_LOCAL__ 0xace8761d
#define LOG_TAG "android_surface_consumer"

typedef struct render_entry_S {
	uint64_t dummy;
	mlt_frame frame;
} render_entry_t;

typedef struct render_queue_S {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	render_entry_t queue[5];
	pthread_t thr_hd;

	int w_index;
	int r_index;

	int ctrl_stop:1;
	int status_stop:1;
	int running:1;
} render_queue_t;

static void queue_thread_start(render_queue_t* queue, void* arg, void* (*thr)(void*))
{
	pthread_create(&queue->thr_hd, NULL, thr, arg);
	pthread_mutex_lock(&queue->mutex);
	while(queue->running == 0) {
		pthread_cond_wait(&queue->cond, &queue->mutex);
	}
	pthread_mutex_unlock(&queue->mutex);
}

static void queue_thread_startup(render_queue_t* queue)
{
	pthread_mutex_lock(&queue->mutex);
	queue->running = 1;
	pthread_mutex_unlock(&queue->mutex);
	pthread_cond_signal(&queue->cond);
}

static void queue_init(render_queue_t* queue)
{
	memset(queue,0x00,sizeof(render_queue_t));
	pthread_mutex_init(&queue->mutex,NULL);
	pthread_cond_init(&queue->cond, NULL);
}

static render_entry_t* rget_queue_entry(render_queue_t* queue, int*);
static void queue_destroy(render_queue_t* queue)
{
	render_entry_t* entry;
	do {
		entry = rget_queue_entry(queue, NULL);
		mlt_frame_close(entry->frame);
	}while (entry != NULL);

	pthread_mutex_destroy(&queue->mutex);
	pthread_cond_destroy(&queue->cond);
}

static render_entry_t* rget_queue_entry(render_queue_t* queue, int* stopped)
{
	pthread_mutex_lock(&queue->mutex);
	while( queue->r_index == queue->w_index && queue->ctrl_stop == 0) {
		struct timespec tm;
		struct timeval tv;
		gettimeofday(&tv,NULL);
		if ( tv.tv_usec + 20000 >= 1000000 ) {
			tm.tv_sec = tv.tv_sec + 1;
			tm.tv_nsec = (tv.tv_usec+20000 - 1000000) * 1000;
		}
		else {
			tm.tv_sec = tv.tv_sec;
			tm.tv_nsec = (tv.tv_usec + 20000) * 1000;
		}
		pthread_cond_timedwait(&queue->cond,&queue->mutex,&tm);
	}

	if (stopped) {
		*stopped = queue->ctrl_stop;
	}

	if ( queue->w_index == queue->r_index) {
		queue->w_index = queue->r_index = 0;
		pthread_mutex_unlock(&queue->mutex);
		return NULL;
	}

	render_entry_t* ret = queue->queue + queue->r_index;
	queue->r_index += 1;
	queue->r_index %= 5;
	pthread_cond_signal(&queue->cond);
	pthread_mutex_unlock(&queue->mutex);
	return ret;
}

static void wqueue_entry(render_queue_t* queue, mlt_frame frame, int audio)
{
	pthread_mutex_lock(&queue->mutex);
	int try_write= 0;
	int try_cnt = 0;
	while(queue->ctrl_stop == 0) {
		try_write = (queue->w_index + 1) % 5;
		if (try_write == queue->r_index) {
			struct timespec tm;
			struct timeval tv;
			gettimeofday(&tv,NULL);
			try_cnt++;
			if ( tv.tv_usec + 20000 >= 1000000 ) {
				tm.tv_sec = tv.tv_sec + 1;
				tm.tv_nsec = (tv.tv_usec + 20000 - 1000000) * 1000;
			}
			else {
				tm.tv_sec = tv.tv_sec;
				tm.tv_nsec = (tv.tv_usec + 20000) * 1000;
			}
			pthread_cond_timedwait(&queue->cond,&queue->mutex,&tm);
			if (try_cnt >= 5) {
				pthread_mutex_unlock(&queue->mutex);
				return;
			}
		}
		else {
			render_entry_t* wo = queue->queue + queue->w_index;
			queue->w_index = try_write;

			if (audio) {
				wo->frame = mlt_frame_clone_audio(frame);
			}
			else {
				wo->frame = mlt_frame_clone_video(frame);
			}
			pthread_mutex_unlock(&queue->mutex);
			pthread_cond_signal(&queue->cond);
			return;
		}
	}

	pthread_mutex_unlock(&queue->mutex);
	return ;
}


static void queue_stop(render_queue_t* queue)
{
	pthread_mutex_lock(&queue->mutex);
	queue->ctrl_stop=1;
	pthread_cond_signal(&queue->cond);
	pthread_mutex_unlock(&queue->mutex);

	pthread_mutex_lock(&queue->mutex);
	while(queue->status_stop == 0) {
		pthread_cond_wait(&queue->cond,&queue->mutex);
	}
	pthread_mutex_unlock(&queue->mutex);
}

typedef struct consumer_local_S {
	uint32_t _magic;
	struct mlt_consumer_s parent;
	render_queue_t video_queue;
	render_queue_t audio_queue;

	pthread_mutex_t run_lock;
	pthread_cond_t run_cond;
	pthread_t thr_hd;

	pthread_mutex_t window_lock;
	ANativeWindow *native_vout;
	int window_width;
	int window_height;
	int window_format;

	int audio_info_fd;
	int video_info_fd;

	uint64_t start_position;
	uint64_t start_time;
	uint64_t elapse_time;
	double fps;
	uint64_t delay_threshold; //队列renderer有延迟时，在此延迟范围内可以逐个render，否则跳过若干帧

	int play_speed; //播放速度，默认1.0.
	int process_preview:1; //以最大速度进行数据处理时的preview
	int play_preview:1;

	int edit_preview:1;

	int ctrl_stop:1;
	int status_stopped:1;
	int running:1;
} consumer_local_t;

static uint64_t gettime(const struct timeval* tv)
{
	return (uint64_t)tv->tv_sec * tv->tv_usec*1000000;
}

static void* video_thread( void* arg );
static void* audio_thread( void* arg );

static void* consumer_thread( void *arg )
{
	consumer_local_t* local = (consumer_local_t*)arg;
	pthread_mutex_lock(&local->run_lock);
	local->running = -1;
	local->thr_hd = pthread_self();
	local->ctrl_stop = 0;
	local->status_stopped = 0;
	queue_thread_start(&local->video_queue, local, video_thread);
	queue_thread_start(&local->audio_queue, local, audio_thread);
	pthread_mutex_unlock(&local->run_lock);
	pthread_cond_signal(&local->run_cond);

	double fps = local->fps;

	int is_last = 0;
	int stop = 0;
	uint64_t next_time = 0;
	while(1) {
		pthread_mutex_lock(&local->run_lock);
		if (next_time != 0) {
			struct timespec tmw = {next_time/1000000,
					(next_time%1000000)*1000};
			pthread_cond_timedwait(&local->run_cond,&local->run_lock, &tmw);
		}
		stop = local->ctrl_stop;
		pthread_mutex_unlock(&local->run_lock);

		if ( stop || is_last ) {
			break;
		}

		mlt_frame frame = mlt_consumer_rt_frame(&local->parent);
		is_last = mlt_properties_get_double(MLT_FRAME_PROPERTIES(frame),
			"_speed") == 0.0;
		mlt_properties frame_props = MLT_FRAME_PROPERTIES(frame);
		struct timeval curtv ;
		gettimeofday(&curtv, NULL);
		uint64_t curtm = gettime(&curtv);
		if (local->start_position==0xffffffff) {
			local->start_position = mlt_properties_get_position(frame_props,"_position");
			local->start_time = curtm;
		}
		else {
			local->elapse_time = curtm - local->start_time;
		}

		wqueue_entry(&local->audio_queue,frame,1);
		wqueue_entry(&local->video_queue,frame,0);
		next_time = curtm + (1000000/fps)/local->play_speed;
	}

	queue_stop(&local->video_queue);
	queue_stop(&local->audio_queue);

	queue_destroy(&local->video_queue);
	queue_destroy(&local->audio_queue);
	pthread_mutex_lock(&local->run_lock);
	local->status_stopped = 1;
	pthread_mutex_unlock(&local->run_lock);
	pthread_cond_signal(&local->run_cond);

	return NULL;
}

static void* video_thread( void* arg )
{
	consumer_local_t* local = (consumer_local_t*)arg;
	queue_thread_startup(&local->video_queue);
	render_queue_t* queue = &local->video_queue;

	uint8_t* image_raw;
	mlt_image_format fmt;
	int image_w,image_h;
	int position;
	mlt_properties frame_props;
	while(1) {
		int is_stop = 0;
		render_entry_t* entry = rget_queue_entry(&local->video_queue,&is_stop);
		frame_props = mlt_frame_properties(entry->frame);
		position = mlt_properties_get_position(frame_props, "_position");
		mlt_frame_get_image(entry->frame,&image_raw,&fmt,&image_w,&image_h,0);

		if ( local->video_info_fd != -1 ) {
			char info_buf[1024];
			size_t sz = snprintf(info_buf,sizeof(info_buf),"img frame:%d raw:%p fmt:%d %dx%d\n",
				position, image_raw, fmt, image_w, image_h);

			write(local->video_info_fd, info_buf, sz);
		}

		mlt_frame_close(entry->frame);
		if ( is_stop)
			break;
	}

	render_entry_t* entry;
	do {
		entry = rget_queue_entry(queue, NULL);
		mlt_frame_close(entry->frame);
	}while (entry != NULL);

	pthread_mutex_lock(&queue->mutex);
	queue->status_stop = 1;
	pthread_mutex_unlock(&queue->mutex);
	pthread_cond_signal(&queue->cond);
	return NULL;
}

static void* audio_thread( void* arg )
{
	consumer_local_t* local = (consumer_local_t*)arg;
	queue_thread_startup(&local->audio_queue);
	render_queue_t* queue = &local->audio_queue;

	void* audio_raw;
	mlt_audio_format fmt;
	int freqs,chnls,samples;
	int position;
	mlt_properties frame_props;

	while(1) {
		int is_stop = 0;
		render_entry_t* entry = rget_queue_entry(&local->audio_queue,&is_stop);

		mlt_frame_get_audio(entry->frame,&audio_raw, &fmt, &freqs,&chnls,&samples);
		frame_props = mlt_frame_properties(entry->frame);
		position = mlt_properties_get_position(frame_props, "_position");

		if ( local->audio_info_fd != -1 ) {
			char info_buf[1024];
			size_t sz = snprintf(info_buf,sizeof(info_buf),"audio frame:%d raw:%p fmt:%d freq:%d ch:%d samples:%d\n",
				position, audio_raw, fmt, freqs, chnls, samples);

			write(local->audio_info_fd, info_buf, sz);
		}

		mlt_frame_close(entry->frame);
		if ( is_stop)
			break;
	}

	render_entry_t* entry;
	do {
		entry = rget_queue_entry(queue, NULL);
		mlt_frame_close(entry->frame);
	}while (entry != NULL);

	pthread_mutex_lock(&queue->mutex);
	queue->status_stop = 1;
	pthread_mutex_unlock(&queue->mutex);
	pthread_cond_signal(&queue->cond);
	return NULL;
}

int mlt_apreview_consumer_vout_created(mlt_consumer obj, JNIEnv* env, jobject out)
{
	if (obj == NULL || out == NULL)
		return -1;
	consumer_local_t *local_obj = (consumer_local_t*)obj->local;
	if ( local_obj == NULL || local_obj->_magic != __MAGIC_LOCAL__ ) {
		return -1;
	}

	pthread_mutex_lock(&local_obj->window_lock);
	ANativeWindow* _tmp_out = ANativeWindow_fromSurface(env, out);
	if (_tmp_out == NULL) {
		pthread_mutex_unlock(&local_obj->window_lock);
		return -1;
	}
	if (local_obj->native_vout) {
		ANativeWindow_release(local_obj->native_vout);
	}
	ANativeWindow_acquire(_tmp_out);
	local_obj->native_vout = _tmp_out;
	pthread_mutex_unlock(&local_obj->window_lock);
	return 0;
}

int mlt_apreview_consumer_vout_destroyed(mlt_consumer obj)
{
	if (obj == NULL)
		return -1;
	consumer_local_t *local_obj = (consumer_local_t*)obj->local;
	if ( local_obj == NULL || local_obj->_magic != __MAGIC_LOCAL__ ) {
		return -1;
	}
	pthread_mutex_lock(&local_obj->window_lock);
	if (local_obj->native_vout)
		ANativeWindow_release(local_obj->native_vout);

	local_obj->native_vout = NULL;
	pthread_mutex_unlock(&local_obj->window_lock);
	return 0;
}

static int consumer_start(mlt_consumer consumer)
{
	consumer_local_t* local = consumer->child;
	mlt_properties properties = mlt_consumer_properties(consumer);

	pthread_mutex_lock(&local->run_lock);
	if ( local->running ) {
		mlt_log_error(mlt_consumer_service(consumer),
			"consumer already runned");
		pthread_mutex_unlock(&local->run_lock);
		return -1;
	}
	pthread_mutex_unlock(&local->run_lock);

	if (mlt_properties_have(properties, "audio_info_fd")) {
		local->audio_info_fd = mlt_properties_get_int(properties, "audio_info_fd");
	}

	if (mlt_properties_have(properties, "video_info_fd")) {
		local->video_info_fd = mlt_properties_get_int(properties, "video_info_fd");
	}

	int render_prepared = 0;
	pthread_mutex_lock(&local->window_lock);
	if ( local->native_vout )
		render_prepared = 1;
	pthread_mutex_unlock(&local->window_lock);

	if (render_prepared == 0) {
		mlt_log_error(mlt_consumer_service(consumer),
			"rendering not prepared");
		return -1;
	}

	if ( 0 != pthread_create(&local->thr_hd,NULL, consumer_thread, local) ) {
		mlt_log_error(mlt_consumer_service(consumer),
			"consumer thread start failed");
		return -1;;
	}
	pthread_mutex_lock(&local->run_lock);
	while(local->running == 0) {
		pthread_cond_wait(&local->run_cond, &local->run_lock);
	}
	pthread_mutex_unlock(&local->run_lock);

	 return 0;
}

static int consumer_stop(mlt_consumer consumer)
{
	consumer_local_t* local = (consumer_local_t*)consumer->child;
	pthread_mutex_lock(&local->run_lock);
	local->ctrl_stop = 1;
	pthread_cond_signal(&local->run_cond);
	pthread_mutex_unlock(&local->run_lock);
	return 0;
}

static int consumer_is_stopped(mlt_consumer consumer)
{
	int ret = 0;
	consumer_local_t* local = (consumer_local_t*)consumer->child;
	pthread_mutex_lock(&local->run_lock);
	ret = local->status_stopped != 0;
	pthread_mutex_unlock(&local->run_lock);
	return ret;
}

static void consumer_purge(mlt_consumer consumer)
{
	return;
}

static void consumer_close(mlt_consumer consumer)
{
	consumer_local_t* local = (consumer_local_t*)consumer->child;
	pthread_mutex_lock(&local->run_lock);
	if (local->running) {
		if (local->ctrl_stop==0) {
			local->ctrl_stop = -1;
			pthread_cond_signal(&local->run_cond);
		}
	}
	pthread_mutex_unlock(&local->run_lock);

	while(consumer_is_stopped(consumer) == 0)
		continue;

	pthread_mutex_destroy(&local->run_lock);
	pthread_cond_destroy(&local->run_cond);

	mlt_apreview_consumer_vout_destroyed(consumer);
}

mlt_consumer consumer_apreview_init( mlt_profile profile, mlt_service_type type, const char *id, char *arg )
{
	consumer_local_t *obj = (consumer_local_t*)calloc(1,sizeof(consumer_local_t));
	if (!obj) return NULL;
	obj->_magic = __MAGIC_LOCAL__;
	if ( mlt_consumer_init(&obj->parent, obj, profile) ) {
		free(obj);
		return NULL;
	}
	mlt_properties properties = mlt_consumer_properties(&obj->parent);

	queue_init(&obj->audio_queue);
	queue_init(&obj->video_queue);

	pthread_mutex_init(&obj->run_lock,NULL);
	pthread_cond_init(&obj->run_cond,NULL);
	pthread_mutex_init(&obj->window_lock,NULL);

	obj->fps = mlt_properties_get_double(properties,"fps");
	obj->delay_threshold = 20000;
	obj->play_speed = 1;
	obj->play_preview = 1;

	obj->status_stopped = 1;

	obj->start_position = 0xffffffff;

	obj->parent.start = consumer_start;
	obj->parent.stop = consumer_stop;
	obj->parent.is_stopped = consumer_is_stopped;
	obj->parent.purge = consumer_purge;
	obj->parent.close = consumer_close;

	if ( arg ) {
		int flg = atoi(arg);
		if (flg < 0) {
			obj->process_preview = 1;
			obj->play_preview = 0;
		}
		else if ( flg > 1 ) {
			obj->process_preview = 0;
			obj->play_preview = 1;
			obj->play_speed = flg;
		}
	}

	mlt_properties_set_double(properties, "volume", 1.0);
	mlt_properties_set( properties, "rescale", "nearest" );
	mlt_properties_set( properties, "deinterlace_method", "onefield" );
	mlt_properties_set_int( properties, "top_field_first", -1 );
	mlt_properties_set_int( properties, "buffer", 1 );

	obj->audio_info_fd = -1;
	obj->video_info_fd = -1;

	return &obj->parent;

}
