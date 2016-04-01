/*
 * mlt_android_audiotrack.h
 *
 *  Created on: 2016��2��15��
 *      Author: li.lei
 */

#ifndef MLT_ANDROID_AUDIOTRACK_H_
#define MLT_ANDROID_AUDIOTRACK_H_
#ifdef __ANDROID__
#include <stdbool.h>
#include <stdint.h>
#include <jni.h>

bool mlt_android_audiotrack_jni_init();
void mlt_android_audiotrack_jni_clean();

typedef struct mlt_android_audiotrack mlt_android_audiotrack;

typedef enum StreamType {
    STREAM_VOICE_CALL = 0,
    STREAM_SYSTEM = 1,
    STREAM_RING = 2,
    STREAM_MUSIC = 3,
    STREAM_ALARM = 4,
    STREAM_NOTIFICATION = 5,
} StreamType;

typedef enum ChannelConfig {
	CHANNEL_OUT_INVALID = 0x0,
	CHANNEL_OUT_DEFAULT = 0x1, /* f-l */
	CHANNEL_OUT_MONO = 0x4, /* f-l, f-r */
	CHANNEL_OUT_STEREO = 0xc, /* f-l, f-r, b-l, b-r */
	CHANNEL_OUT_QUAD = 0xcc, /* f-l, f-r, b-l, b-r */
	CHANNEL_OUT_SURROUND = 0x41c, /* f-l, f-r, f-c, b-c */
	CHANNEL_OUT_5POINT1 = 0xfc, /* f-l, f-r, b-l, b-r, f-c, low */
	CHANNEL_OUT_7POINT1 = 0x3fc, /* f-l, f-r, b-l, b-r, f-c, low, f-lc, f-rc */

	CHANNEL_OUT_FRONT_LEFT = 0x4,
	CHANNEL_OUT_FRONT_RIGHT = 0x8,
	CHANNEL_OUT_BACK_LEFT = 0x40,
	CHANNEL_OUT_BACK_RIGHT = 0x80,
	CHANNEL_OUT_FRONT_CENTER = 0x10,
	CHANNEL_OUT_LOW_FREQUENCY = 0x20,
	CHANNEL_OUT_FRONT_LEFT_OF_CENTER = 0x100,
	CHANNEL_OUT_FRONT_RIGHT_OF_CENTER = 0x200,
	CHANNEL_OUT_BACK_CENTER = 0x400,
}ChannelConfig;

typedef enum AudioFormat {
    ENCODING_INVALID = 0,
    ENCODING_DEFAULT = 1,
    ENCODING_PCM_16BIT = 2, // signed, guaranteed to be supported by devices.
    ENCODING_PCM_8BIT = 3/*, // unsigned, not guaranteed to be supported by devices.
    ENCODING_PCM_FLOAT = 4, // single-precision floating-point per sample*/
} AudioFormat;

typedef struct mlt_android_audiotrack_spec {
    int sample_rate_in_hz;
    StreamType stream_type;
    ChannelConfig channel_config;

    AudioFormat audio_format;
    int buffer_size_in_bytes;

    enum Mode {
        MODE_STATIC = 0,
        MODE_STREAM = 1,
    } mode;

    enum WriteMode {
        WRITE_BLOCKING     = 0,
        WRITE_NON_BLOCKING = 1,
    } write_mode; // not used

    // extra field
    int sdl_samples;
} mlt_android_audiotrack_spec;

mlt_android_audiotrack* mlt_android_audiotrack_new(JNIEnv* evn, mlt_android_audiotrack_spec* spec);
void mlt_android_audiotrack_free(JNIEnv* env, mlt_android_audiotrack* obj);

//void mlt_android_audiotrack_get_target_spec(mlt_android_audiotrack* atrack, SDL_AudioSpec *spec);
int mlt_android_audiotrack_get_min_buffer_size(mlt_android_audiotrack* atrack);
int mlt_android_audiotrack_get_native_output_sample_rate(JNIEnv *env, StreamType type/* = NULL */);

void mlt_android_audiotrack_play(JNIEnv *env, mlt_android_audiotrack *atrack);
void mlt_android_audiotrack_pause(JNIEnv *env, mlt_android_audiotrack *atrack);
void mlt_android_audiotrack_flush(JNIEnv *env, mlt_android_audiotrack *atrack);
void mlt_android_audiotrack_set_volume(JNIEnv *env, mlt_android_audiotrack *atrack, float left_volume, float right_volume);
void mlt_android_audiotrack_stop(JNIEnv *env, mlt_android_audiotrack *atrack);
void mlt_android_audiotrack_release(JNIEnv *env, mlt_android_audiotrack *atrack);
int mlt_android_audiotrack_write(JNIEnv *env, mlt_android_audiotrack *atrack, uint8_t *data, int size_in_byte);

int mlt_android_audiotrack_getAudioSessionId(JNIEnv *env, mlt_android_audiotrack *atrack);

#endif
#endif /* SRC_FRAMEWORK_MLT_ANDROID_AUDIOTRACK_H_ */
