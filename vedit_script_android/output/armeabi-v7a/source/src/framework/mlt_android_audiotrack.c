/*
 * mlt_android_audiotrack.c
 *
 *  Created on: 2016��2��15��
 *      Author: li.lei
 *
 */

#ifdef __ANDROID__
#include "mlt_android_audiotrack.h"
#include "mlt_android_jni_util.h"
#include "mlt_android_env.h"

static struct {
	jclass clazz;
    jmethodID constructor;
    jmethodID getMinBufferSize;
    jmethodID getMaxVolume;
    jmethodID getMinVolume;
    jmethodID getNativeOutputSampleRate;

    jmethodID play;
    jmethodID pause;
    jmethodID flush;
    jmethodID stop;
    jmethodID release;
    jmethodID write_byte;
    jmethodID setStereoVolume;

    jmethodID write_float;

    jmethodID getAudioSessionId;

} gAudioTrackJni;

bool mlt_android_audiotrack_jni_init()
{
	JNIEnv* env = mlt_android_get_jnienv();
	FIND_JAVA_CLASS(env, gAudioTrackJni.clazz, "android/media/AudioTrack", false);
	FIND_JAVA_METHOD(env, gAudioTrackJni.constructor, gAudioTrackJni.clazz, "<init>", "(IIIIII)V",false);
	FIND_JAVA_STATIC_METHOD(env, gAudioTrackJni.getMinBufferSize, gAudioTrackJni.clazz, "getMinBufferSize", "(III)I", false);
	FIND_JAVA_STATIC_METHOD(env, gAudioTrackJni.getMaxVolume, gAudioTrackJni.clazz, "getMaxVolume", "()F", false);
	FIND_JAVA_STATIC_METHOD(env, gAudioTrackJni.getMinVolume, gAudioTrackJni.clazz, "getMinVolume", "()F", false);
	FIND_JAVA_STATIC_METHOD(env, gAudioTrackJni.getNativeOutputSampleRate, gAudioTrackJni.clazz, "getNativeOutputSampleRate", "(I)I", false);

	FIND_JAVA_METHOD(env, gAudioTrackJni.play, gAudioTrackJni.clazz, "play", "()V", false);
	FIND_JAVA_METHOD(env, gAudioTrackJni.pause, gAudioTrackJni.clazz, "pause", "()V", false);
	FIND_JAVA_METHOD(env, gAudioTrackJni.flush, gAudioTrackJni.clazz, "flush", "()V", false);
	FIND_JAVA_METHOD(env, gAudioTrackJni.stop, gAudioTrackJni.clazz, "stop", "()V", false);
	FIND_JAVA_METHOD(env, gAudioTrackJni.release, gAudioTrackJni.clazz, "release", "()V", false);

	FIND_JAVA_METHOD(env, gAudioTrackJni.write_byte, gAudioTrackJni.clazz, "write", "([BII)I", false);
	FIND_JAVA_METHOD(env, gAudioTrackJni.setStereoVolume, gAudioTrackJni.clazz, "setStereoVolume", "(FF)I", false);
	FIND_JAVA_METHOD(env, gAudioTrackJni.getAudioSessionId, gAudioTrackJni.clazz, "getAudioSessionId", "()I", false);
	return true;
}

void mlt_android_audiotrack_jni_clean()
{

}

static int audiotrack_get_min_buffer_size(JNIEnv *env, mlt_android_audiotrack_spec *spec)
{
    int retval = (*env)->CallStaticIntMethod(env, gAudioTrackJni.clazz, gAudioTrackJni.getMinBufferSize,
        (int) spec->sample_rate_in_hz,
        (int) spec->channel_config,
        (int) spec->audio_format);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("audiotrack_get_min_buffer_size: getMinBufferSize: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return -1;
    }

    return retval;
}


static float audiotrack_get_max_volume(JNIEnv *env)
{

    float retval = (*env)->CallStaticFloatMethod(env, gAudioTrackJni.clazz, gAudioTrackJni.getMaxVolume);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("audiotrack_get_max_volume: getMaxVolume: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return -1;
    }

    return retval;
}

static float audiotrack_get_min_volume(JNIEnv *env)
{

    float retval = (*env)->CallStaticFloatMethod(env, gAudioTrackJni.clazz, gAudioTrackJni.getMinVolume);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("audiotrack_get_min_volume: getMinVolume: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return -1;
    }

    return retval;
}

typedef struct mlt_android_audiotrack {
    jobject thiz;

    mlt_android_audiotrack_spec spec;

    jbyteArray  byte_buffer;
    int         byte_buffer_capacity;
    int         min_buffer_size;
    float       max_volume;
    float       min_volume;

    jfloatArray float_buffer;
    int         float_buffer_capacity;
} mlt_android_audiotrack;

typedef mlt_android_audiotrack SDL_Android_AudioTrack;

static void SDL_Android_AudioTrack_get_default_spec(mlt_android_audiotrack_spec *spec)
{
    spec->stream_type = STREAM_MUSIC;
    spec->sample_rate_in_hz = 0;
    spec->channel_config = CHANNEL_OUT_STEREO;
    spec->audio_format = ENCODING_PCM_16BIT;
    spec->buffer_size_in_bytes = 0;
    spec->mode = MODE_STREAM;
}

int mlt_android_audiotrack_get_native_output_sample_rate(JNIEnv *env, StreamType type/* = NULL */)
{
    int retval = (*env)->CallStaticIntMethod(env, gAudioTrackJni.clazz, gAudioTrackJni.getNativeOutputSampleRate, type);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("audiotrack_get_native_output_sample_rate: getMinVolume: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return -1;
    }

    return retval;
}

static int audiotrack_set_stereo_volume(JNIEnv *env, SDL_Android_AudioTrack *atrack, float left, float right)
{
    int retval = (*env)->CallIntMethod(env, atrack->thiz, gAudioTrackJni.setStereoVolume, left, right);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("audiotrack_set_stereo_volume: write_byte: Exception:");
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }
        return -1;
    }

    return retval;
}

void mlt_android_audiotrack_set_volume(JNIEnv *env, SDL_Android_AudioTrack *atrack, float left_volume, float right_volume)
{
    audiotrack_set_stereo_volume(env, atrack, left_volume, right_volume);
}

mlt_android_audiotrack* mlt_android_audiotrack_new(JNIEnv* env, mlt_android_audiotrack_spec* spec)
{
    //jint sdk_int = mlt_android_sdk_version();

    switch (spec->channel_config) {
    case CHANNEL_OUT_MONO:
        ALOGI("mlt_android_audiotrack: %s", "CHANNEL_OUT_MONO");
        break;
    case CHANNEL_OUT_STEREO:
        ALOGI("mlt_android_audiotrack: %s", "CHANNEL_OUT_STEREO");
        break;
    default:
        ALOGE("mlt_android_audiotrack_new_from_spec: invalid channel %d", spec->channel_config);
        return NULL;
    }

    switch (spec->audio_format) {
    case ENCODING_PCM_16BIT:
        ALOGI("mlt_android_audiotrack: %s", "ENCODING_PCM_16BIT");
        break;
    case ENCODING_PCM_8BIT:
        ALOGI("mlt_android_audiotrack: %s", "ENCODING_PCM_8BIT");
        break;
    /*case ENCODING_PCM_FLOAT:
        ALOGI("mlt_android_audiotrack: %s", "ENCODING_PCM_FLOAT");
        if (sdk_int < 21) {
            ALOGI("mlt_android_audiotrack: %s need API 21 or above", "ENCODING_PCM_FLOAT");
            return NULL;
        }
        break;*/
    default:
        ALOGE("mlt_android_audiotrack_new_from_spec: invalid format %d", spec->audio_format);
        return NULL;
    }

    SDL_Android_AudioTrack *atrack = (SDL_Android_AudioTrack*) calloc(1,sizeof(SDL_Android_AudioTrack));
    if (!atrack) {
        (*env)->CallVoidMethod(env, atrack->thiz, gAudioTrackJni.release);
        return NULL;
    }
    atrack->spec = *spec;

    if (atrack->spec.sample_rate_in_hz < 4000 || atrack->spec.sample_rate_in_hz > 48000) {
        int native_sample_rate_in_hz = mlt_android_audiotrack_get_native_output_sample_rate(env, atrack->spec.stream_type);
        if (native_sample_rate_in_hz > 0) {
            ALOGE("mlt_android_audiotrack_new: cast sample rate %d to %d:",
                atrack->spec.sample_rate_in_hz,
                native_sample_rate_in_hz);
            atrack->spec.sample_rate_in_hz = native_sample_rate_in_hz;
        }
    }

    int min_buffer_size = audiotrack_get_min_buffer_size(env, &atrack->spec);
    if (min_buffer_size <= 0) {
        ALOGE("mlt_android_audiotrack_new: mlt_android_audiotrack_get_min_buffer_size: return %d:", min_buffer_size);
        free(atrack);
        return NULL;
    }

    jobject thiz = (*env)->NewObject(env, gAudioTrackJni.clazz, gAudioTrackJni.constructor,
        (int) atrack->spec.stream_type,
        (int) atrack->spec.sample_rate_in_hz,
        (int) atrack->spec.channel_config,
        (int) atrack->spec.audio_format,
        (int) min_buffer_size,
        (int) atrack->spec.mode);
    if (!thiz || (*env)->ExceptionCheck(env)) {
        ALOGE("mlt_android_audiotrack_new: NewObject: Exception:");
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }
        free(atrack);
        return NULL;
    }

    atrack->min_buffer_size = min_buffer_size;
    atrack->spec.buffer_size_in_bytes = min_buffer_size;
    atrack->max_volume = audiotrack_get_max_volume(env);
    atrack->min_volume = audiotrack_get_min_volume(env);

    atrack->thiz = (*env)->NewGlobalRef(env, thiz);
    (*env)->DeleteLocalRef(env, thiz);
#ifndef IJKMAX
#define IJKMAX(a, b)    ((a) > (b) ? (a) : (b))
#endif

#ifndef IJKMIN
#define IJKMIN(a, b)    ((a) < (b) ? (a) : (b))
#endif
    // extra init
    float init_volume = 1.0f;
    init_volume = IJKMIN(init_volume, atrack->max_volume);
    init_volume = IJKMAX(init_volume, atrack->min_volume);
    ALOGI("mlt_android_audiotrack_new: init volume as %f/(%f,%f)", init_volume, atrack->min_volume, atrack->max_volume);
    audiotrack_set_stereo_volume(env, atrack, init_volume, init_volume);

    return atrack;
}

void mlt_android_audiotrack_free(JNIEnv *env, mlt_android_audiotrack* atrack)
{
    if (atrack->byte_buffer) {
        (*env)->DeleteGlobalRef(env, atrack->byte_buffer);
        atrack->byte_buffer = NULL;
    }
    atrack->byte_buffer_capacity = 0;

    if (atrack->float_buffer) {
        (*env)->DeleteGlobalRef(env, atrack->float_buffer);
        atrack->float_buffer = NULL;
    }
    atrack->float_buffer_capacity = 0;

    if (atrack->thiz) {
    	mlt_android_audiotrack_release(env, atrack);
        (*env)->DeleteGlobalRef(env, atrack->thiz);
        atrack->thiz = NULL;
    }

    free(atrack);
}

void mlt_android_audiotrack_release(JNIEnv *env, SDL_Android_AudioTrack *atrack)
{

    (*env)->CallVoidMethod(env, atrack->thiz, gAudioTrackJni.release);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("SDL_Android_AudioTrack_release: release: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return;
    }
}

int mlt_android_audiotrack_get_min_buffer_size(SDL_Android_AudioTrack* atrack)
{
    return atrack->min_buffer_size;
}

void mlt_android_audiotrack_play(JNIEnv *env, SDL_Android_AudioTrack *atrack)
{
    (*env)->CallVoidMethod(env, atrack->thiz, gAudioTrackJni.play);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("mlt_android_audiotrack_play: play: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return;
    }
}

void mlt_android_audiotrack_pause(JNIEnv *env, mlt_android_audiotrack *atrack)
{
    (*env)->CallVoidMethod(env, atrack->thiz, gAudioTrackJni.pause);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("mlt_android_audiotrack_pause: pause: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return;
    }
}

void mlt_android_audiotrack_flush(JNIEnv *env, mlt_android_audiotrack *atrack)
{
    (*env)->CallVoidMethod(env, atrack->thiz, gAudioTrackJni.flush);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("mlt_android_audiotrack_flush: flush: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return;
    }
}

void mlt_android_audiotrack_stop(JNIEnv *env, mlt_android_audiotrack *atrack)
{
    (*env)->CallVoidMethod(env, atrack->thiz, gAudioTrackJni.stop);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("mlt_android_audiotrack_stop: stop: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return;
    }
}


static int SDL_Android_AudioTrack_reserve_byte_buffer(JNIEnv *env, SDL_Android_AudioTrack *atrack, int size_in_byte)
{
    if (atrack->byte_buffer && size_in_byte <= atrack->byte_buffer_capacity)
        return size_in_byte;

    if (atrack->byte_buffer) {
        (*env)->DeleteGlobalRef(env, atrack->byte_buffer);
        atrack->byte_buffer = NULL;
        atrack->byte_buffer_capacity = 0;
    }

    int capacity = IJKMAX(size_in_byte, atrack->min_buffer_size);
    jbyteArray byte_buffer = (*env)->NewByteArray(env, capacity);
    if (!byte_buffer || (*env)->ExceptionCheck(env)) {
        ALOGE("%s: NewByteArray: Exception:", __func__);
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }
        return -1;
    }

    atrack->byte_buffer_capacity = capacity;
    atrack->byte_buffer = (*env)->NewGlobalRef(env, byte_buffer);
    (*env)->DeleteLocalRef(env, byte_buffer);
    return capacity;
}

int mlt_android_audiotrack_write(JNIEnv *env, SDL_Android_AudioTrack *atrack, uint8_t *data, int size_in_byte)
{
    if (size_in_byte <= 0)
        return size_in_byte;

    int reserved = SDL_Android_AudioTrack_reserve_byte_buffer(env, atrack, size_in_byte);
    if (reserved < size_in_byte) {
        ALOGE("%s failed %d < %d\n", __FUNCTION__, reserved, size_in_byte);
        return -1;
    }

    (*env)->SetByteArrayRegion(env, atrack->byte_buffer, 0, (int)size_in_byte, (jbyte*) data);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("%s: SetByteArrayRegion: Exception:\n", __func__);
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }
        return -1;
    }

    int retval = (*env)->CallIntMethod(env, atrack->thiz, gAudioTrackJni.write_byte,
        atrack->byte_buffer, 0, (int)size_in_byte);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("%s: write_byte: Exception:\n", __func__);
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }
        return -1;
    }

    return retval;
}


int mlt_android_audiotrack_getAudioSessionId(JNIEnv *env, SDL_Android_AudioTrack *atrack)
{
    int audioSessionId = (*env)->CallIntMethod(env, atrack->thiz, gAudioTrackJni.getAudioSessionId);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("%s: Exception:", __func__);
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return 0;
    }
    return audioSessionId;
}
#endif



