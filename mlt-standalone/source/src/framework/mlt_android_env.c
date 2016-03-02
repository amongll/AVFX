/*
 * mlt_android_env.c
 *
 *  Created on: 2016Äê1ÔÂ18ÈÕ
 *      Author: L-F000000-PC
 */
#ifdef ANDROID
#include "mlt_android_env.h"
#include "mlt_factory.h"
#include <libgen.h>
#include <errno.h>
#include "mlt_log.h"
#include <pthread.h>
#include "mlt_android_jni_util.h"
#include "mlt_android_audiotrack.h"

static JavaVM *android_global_javavm = NULL;

typedef enum property_parse_status_E {
	status_start=0,
	status_key,
	status_subkey,
	status_equal,
	status_value,
	status_comment,
	status_cr,
	status_lb,
	status_invalid
} property_parse_status_e;

typedef struct property_parse_context_s {
	char* buf;
	size_t buf_size;
	int cur;
	int end;

	char* key;
	char* compose_key;
	int compose_main_sz;
	int compose_key_bufsz;
	char* value;
	property_parse_status_e state;
	int v_crlb:1;
	int is_subkey:1;
} property_parse_context_t;

static int parse_property(char* input, size_t size, property_parse_context_t *context)
{
	if ( !context ) return -1;

	if ( size ) {
		if ( size + context->end >= context->buf_size ) {
			context->buf = (char*)realloc( context->buf, size + context->end  + 256);
			context->buf_size = context->end + size + 256;
		}
		memcpy ( context->buf + context->end, input, size);
		context->end += size;
	}

	while( context->cur < context->end ) {
		char p = context->buf[context->cur];
		switch(context->state) {
		case status_invalid:
			return -1;
			break;
		case status_start:
			switch( p ) {
			case '\n':
				context->state = status_lb;
				break;
			case '\r':
				context->state = status_cr;
				break;
			case '#':
				context->state = status_comment;
				break;
			default:
				context->state = status_key;
				context->key = context->buf + context->cur;
				break;
			}

			break;
		case status_key:
			switch ( p ) {
			case '=':
				context->state = status_equal;
				context->buf[context->cur] = '\0';
				break;
			case '\r':
			case '\n':
				context->state = status_invalid;
				return -1;
				break;
			default:
				break;
			}
			break;
		case status_subkey:
			switch (p) {
			case '\r':
			case '\n':
			case '=':
			case '.':
				context->state = status_invalid;
				return -1;
				break;
			default:
				context->state = status_key;
				context->key = context->buf + context->cur - 1;
				break;
			}
			break;
		case status_equal:
			context->value = context->buf + context->cur;
			switch ( p ) {
			case '\r':
				context->v_crlb = 1;
				context->state = status_cr;
				context->buf[context->cur]='\0';
				context->cur++;
				if (context->is_subkey == 0) {
					if (context->compose_key == NULL ||
							context->compose_key_bufsz <= strlen(context->key) ) {
						context->compose_key = (char*)realloc(context->compose_key,
								strlen(context->key)+256);
						context->compose_key_bufsz = strlen(context->key) + 256;
					}
					context->compose_main_sz = strlen(context->key);
					sprintf(context->compose_key,"%s",context->key);
				}
				else {
					if (context->compose_key_bufsz <= strlen(context->key) + context->compose_main_sz) {
						context->compose_key = (char*)realloc(context->compose_key,
							strlen(context->key) + context->compose_main_sz + 256);
						context->compose_key_bufsz = strlen(context->key) +
							context->compose_main_sz + 256;
					}
					sprintf(context->compose_key+context->compose_main_sz,"%s",context->key);
					context->key = context->compose_key;
				}
				return 1;
				break;
			case '\n':
				context->v_crlb = 1;
				context->state = status_lb;
				context->buf[context->cur]='\0';
				context->cur++;
				if (context->is_subkey == 0) {
					if (context->compose_key == NULL ||
							context->compose_key_bufsz <= strlen(context->key) ) {
						context->compose_key = (char*)realloc(context->compose_key,
								strlen(context->key)+256);
						context->compose_key_bufsz = strlen(context->key) + 256;
					}
					context->compose_main_sz = strlen(context->key);
					sprintf(context->compose_key,"%s",context->key);
				}
				else {
					if (context->compose_key_bufsz <= strlen(context->key) + context->compose_main_sz) {
						context->compose_key = (char*)realloc(context->compose_key,
							strlen(context->key) + context->compose_main_sz + 256);
						context->compose_key_bufsz = strlen(context->key) +
							context->compose_main_sz + 256;
					}
					sprintf(context->compose_key+context->compose_main_sz,"%s",context->key);
					context->key = context->compose_key;
				}
				return 1;
				break;
			default:
				context->state = status_value;
				break;
			}
			break;
		case status_value:
			switch ( p ) {
			case '\r':
				context->v_crlb = 1;
				context->state = status_cr;
				context->buf[context->cur]='\0';
				context->cur++;
				if (context->is_subkey == 0) {
					if (context->compose_key == NULL ||
							context->compose_key_bufsz <= strlen(context->key) ) {
						context->compose_key = (char*)realloc(context->compose_key,
								strlen(context->key)+256);
						context->compose_key_bufsz = strlen(context->key) + 256;
					}
					context->compose_main_sz = strlen(context->key);
					sprintf(context->compose_key,"%s",context->key);
				}
				else {
					if (context->compose_key_bufsz <= strlen(context->key) + context->compose_main_sz) {
						context->compose_key = (char*)realloc(context->compose_key,
							strlen(context->key) + context->compose_main_sz + 256);
						context->compose_key_bufsz = strlen(context->key)
							+ context->compose_main_sz + 256;
					}
					sprintf(context->compose_key+context->compose_main_sz,"%s",context->key);
					context->key = context->compose_key;
				}
				return 1;
				break;
			case '\n':
				context->v_crlb = 1;
				context->state = status_lb;
				context->buf[context->cur]='\0';
				context->cur++;
				if (context->is_subkey == 0) {
					if (context->compose_key == NULL ||
							context->compose_key_bufsz <= strlen(context->key) ) {
						context->compose_key = (char*)realloc(context->compose_key,
								strlen(context->key)+256);
						context->compose_key_bufsz = strlen(context->key) + 256;
					}
					context->compose_main_sz = strlen(context->key);
					sprintf(context->compose_key,"%s",context->key);
				}
				else {
					if (context->compose_key_bufsz <= strlen(context->key) + context->compose_main_sz) {
						context->compose_key = (char*)realloc(context->compose_key,
							strlen(context->key) + context->compose_main_sz + 256);
						context->compose_key_bufsz = strlen(context->key)
							+ context->compose_main_sz + 256;
					}
					sprintf(context->compose_key+context->compose_main_sz,"%s",context->key);
					context->key = context->compose_key;
				}
				return 1;
				break;
			default:
				context->state = status_value;
				break;
			}
			break;
		case status_lb:
			switch( p ) {
			case '\n':
				context->v_crlb=0;
				context->state = status_lb;
				break;
			case '\r':
				context->v_crlb=0;
				context->state = status_cr;
				break;
			case '.':
				if (context->v_crlb) {
					context->is_subkey = 1;
					context->state = status_subkey;
				}
				else {
					context->state = status_invalid;
					return -1;
				}
				break;
			case '#':
				context->v_crlb=0;
				context->state = status_comment;
				break;
			default:
				context->v_crlb=0;
				context->is_subkey = 0;
				context->state = status_key;
				context->key = context->buf + context->cur;
				break;
			}
			break;
		case status_cr:
			switch( p ) {
			case '\n':
				context->state = status_lb;
				break;
			default:
				context->state = status_invalid;
				return -1;
				break;
			}
			break;
		case status_comment:
			switch(p) {
			case '\n':
				context->state = status_lb;
				break;
			case '\r':
				context->state = status_cr;
				break;
			default:
				break;
			}
			break;
		}
		context->cur++;
	}
	return 0;
}

static void step_parse_context(property_parse_context_t* ctx)
{
	if (ctx->cur == 0)return;
	if (ctx->cur < ctx->end)
		memmove(ctx->buf, ctx->buf + ctx->cur, ctx->end - ctx->cur);
	ctx->end -= ctx->cur;
	ctx->cur = 0;
	ctx->key = NULL;
	ctx->value = NULL;
}

static void init_parse_context(property_parse_context_t* ctx)
{
	memset(ctx, 0x00, sizeof(property_parse_context_t));
}

static void cleanup_parse_context(property_parse_context_t* ctx)
{
	if (!ctx) return;
	if (ctx->compose_key)free(ctx->compose_key);
	memset(ctx, 0x00, sizeof(property_parse_context_t));
}

static int load_properties(AAssetManager* mgr, mlt_properties self, const char *name)
{
	// Convert filename string encoding.
	//mlt_properties_set( self, "_mlt_properties_load", name );
	//mlt_properties_from_utf8( self, "_mlt_properties_load", "__mlt_properties_load" );
	//name = mlt_properties_get( self, "__mlt_properties_load" );

	// Open the file
	AAsset* file = AAssetManager_open(mgr, name, AASSET_MODE_STREAMING );
	if ( !file) {
		return -1;
	}

	int ret = 0;
	char temp[ 1024 ];
	property_parse_context_t ctx;
	memset(&ctx,0x00,sizeof(property_parse_context_t));

	do {
		ret = AAsset_read(file, temp, sizeof(temp));
		if ( ret < 0 ) {
			break;
		}
		else if (ret > 0 ) {
			int have = parse_property(temp, ret, &ctx);
			while(have == 1) {
				mlt_properties_set(self, ctx.key, ctx.value);
				step_parse_context(&ctx);
				have = parse_property(NULL, 0, &ctx);
			}
			if (have == -1) {
				AAsset_close(file);
				return -1;
			}
		}
	} while( ret > 0);

	if ( ctx.state == status_value && ctx.key && ctx.value ) {
		mlt_properties_set(self, ctx.key, ctx.value);
	}

	cleanup_parse_context(&ctx);

	// Close the file
	AAsset_close(file);
	return ret;
}


mlt_properties mlt_properties_load_AAsset(AAssetManager* mgr, const char *assetName )
{
	mlt_properties self = mlt_properties_new( );

	if ( self != NULL )
		load_properties(mgr, self, assetName);

	// Return the pointer
	return self;
}

int mlt_properties_preset_AAsset(AAssetManager* mgr, mlt_properties self, const char *assetName)
{
	// Look for profile-specific preset before a generic one.
	const char *data    = "mlt/presets";
	const char *type    = mlt_properties_get( self, "mlt_type" );
	const char *service = mlt_properties_get( self, "mlt_service" );
	const char *profile = mlt_environment( "MLT_PROFILE" );
	int error = 0;

	if ( data && type && service )
	{
		char *path = malloc( 5 + strlen(assetName) + strlen(data) + strlen(type) + strlen(service) + ( profile? strlen(profile) : 0 ) );
		sprintf( path, "%s/%s/%s/%s/%s", data, type, service, profile, assetName );
		if ( load_properties( mgr, self, path ) )
		{
			sprintf( path, "%s/%s/%s/%s", data, type, service, assetName );
			error = load_properties( mgr, self, path );
		}
		free( path );
	}
	else
	{
		error = 1;
	}
	return error;
}

static bool my_get_dir(const char* dir_name);

static bool my_get_dir_for_file(const char* file)
{
	char buf[1024];
	int len = snprintf(buf, sizeof(buf), "%s", file);
	int prev_del = -1, i;
	for ( i = len - 1; i >= 0 ; i--) {
		if ( buf[i] == '/' && (prev_del == -1 || i == prev_del - 1) ) {
			buf[i] = '\0';
			prev_del = i;
		}
		else if (prev_del != -1)
			break;
	}

	return my_get_dir(buf);
}

static bool my_get_dir(const char* dir_name)
{
	char buf[1024];
	int len = snprintf(buf, sizeof(buf), "%s", dir_name);
	int prev_del = -1, i;
	for ( i = len - 1; i >= 0 ; i--) {
		if ( buf[i] == '/' && (prev_del == -1 || i == prev_del - 1) ) {
			buf[i] = '\0';
			prev_del = i;
		}
		else if (prev_del != -1)
			break;
	}

	//char* dir = dirname(buf);
	//char* base = basename(buf);

	struct stat statbuf1;
	if ( 0 == stat(dir_name, &statbuf1) ) {
		if ( S_ISDIR(statbuf1.st_mode) ) {
			if ( access(dir_name, R_OK|W_OK|X_OK) == 0)
				return true;
			else
				return false;
		}
		else
			return false;
	}
	else {
		if ( true == my_get_dir(buf) ) {
			if (mkdir(dir_name, S_IRWXU))
				return false;
			return true;
		}
		else
			return false;
	}
}

static bool copy_asset_file(int fd, AAsset* asset)
{
	if( fd==-1 || asset == NULL)return false;
	char tmp[4096];
	int ret = 0;

	do {
		ret = AAsset_read(asset, tmp, sizeof(tmp));
		if ( ret < 0) {
			return false;
		}
		else if (ret == 0)
			break;
		else {
			int _ret = write(fd, tmp, ret);
			if ( _ret != ret) {
				return false;
			}
		}
	} while (ret > 0);
	return true;
}

extern bool mlt_android_check_data(AAssetManager* mgr, const char* files_path, char *err, size_t err_size)
{
	char path[1024];
	char real_path[1024];
	mlt_properties assMd5s = mlt_properties_load_AAsset(mgr, "mlt/md5check.props");
	if (assMd5s == NULL) {
		snprintf(err,err_size, "Asset mlt/md5check.props load failed");
		mlt_log_error(NULL, "%s", err);
		goto failed;
	}

	const char* _fmt = "%s/%s";
	if ( files_path[strlen(files_path)-1] == '/') {
		_fmt = "%s%s";
	}

	int i, count = mlt_properties_count(assMd5s);
	for ( i=0 ; i<count; i++) {
		const char* name = mlt_properties_get_name(assMd5s,i);
		const char* value = mlt_properties_get_value(assMd5s, i);

		memset(path,0x00,sizeof(path));
		memset(real_path,0x00,sizeof(real_path));

		snprintf(path,sizeof(path), _fmt, files_path, name);
		snprintf(real_path,sizeof(real_path), "%s.%s", path, value);

		struct stat statbuf;
		if ( 0 == lstat(path, &statbuf) ) {
			if ( !S_ISLNK(statbuf.st_mode) ) {
				snprintf(err,err_size, "%s not symbol link", path);
				mlt_log_error(NULL, "%s", err);
				goto failed;
			}
			else {
				char check_real[1024];
				ssize_t _ret = readlink(path, check_real,sizeof(check_real));
				if ( _ret > 0 && strncmp( check_real, real_path, _ret) == 0 && !stat(path,&statbuf) ) {
					mlt_log_info(NULL, "mlt presets file: %s for MD5:%s exists",
							path, value);
					continue;
				}
			}
		}
		else {
			if ( false == my_get_dir_for_file(real_path) ) {
				snprintf(err,err_size,"check dir for %s failed, %s", real_path, strerror(errno));
				mlt_log_error(NULL, "%s", err);
				goto failed;
			}
		}

		AAsset* asset = AAssetManager_open(mgr, name, O_RDONLY);
		int fd = open(real_path, O_CREAT|O_TRUNC|O_WRONLY, S_IRWXU);
		if ( false == copy_asset_file(fd, asset) ) {
			if (asset)AAsset_close(asset);
			if (fd != -1)close(fd);
			snprintf(err,err_size,"copy asset failed:%s %s",name, strerror(errno));
			mlt_log_error(NULL, "%s", err);
			goto failed;
		}
		AAsset_close(asset);
		close(fd);

		unlink(path);
		if (symlink(real_path, path) ) {
			snprintf(err, err_size, "symlink %s %s failed:%s", real_path, path, strerror(errno));
			mlt_log_error(NULL, "%s", err);
			goto failed;
		}
	}

	if (assMd5s)mlt_properties_close(assMd5s);
	return true;
failed:
	if (assMd5s)mlt_properties_close(assMd5s);
	return false;
}

static mlt_properties g_aenv = NULL;
static pthread_mutex_t g_aenv_lock = PTHREAD_MUTEX_INITIALIZER;

static pthread_key_t g_aenv_key  ;
static pthread_once_t g_aevn_once = PTHREAD_ONCE_INIT;

static struct ASDKBuildInfo {
	jclass clazz;
	jfieldID jfid_SDK_INT;
} gASDK ;

static void aenv_thr_spec_key_clean(void* value)
{
    JNIEnv *env = (JNIEnv*) value;
    if (env != NULL) {
        (*android_global_javavm)->DetachCurrentThread(android_global_javavm);
        pthread_setspecific(g_aenv_key, NULL);
    }
}
static void aenv_thr_spec_key_create()
{
	pthread_key_create(&g_aenv_key, aenv_thr_spec_key_clean);
}

bool mlt_android_env_init(JavaVM* vm)
{
	if ( g_aenv == NULL) {
		pthread_mutex_lock(&g_aenv_lock);
		if (g_aenv == NULL) {
			g_aenv = mlt_properties_new();
			android_global_javavm = vm;
			JNIEnv* env = NULL;
			if ( (*vm)->GetEnv(vm, (void**)&env,JNI_VERSION_1_4) != JNI_OK )
				return false;

		    FIND_JAVA_CLASS( env, gASDK.clazz, "android/os/Build$VERSION", false);
		    FIND_JAVA_STATIC_FIELD(env, gASDK.jfid_SDK_INT,   gASDK.clazz,
		        "SDK_INT",   "I", false);

		    if ( ! mlt_android_audiotrack_jni_init() ) return false;
		}
		pthread_mutex_unlock(&g_aenv_lock);
	}
	pthread_once(&g_aevn_once, aenv_thr_spec_key_create);
	return true;
}

void mlt_android_env_destroy()
{
	pthread_mutex_lock(&g_aenv_lock);
	if (g_aenv) {
		mlt_properties_dec_ref(g_aenv);
		g_aenv = NULL;
	}
	pthread_mutex_unlock(&g_aenv_lock);
}

JNIEnv* mlt_android_get_jnienv()
{
	JNIEnv* ret = (JNIEnv*)pthread_getspecific(g_aenv_key);
	if (ret) return ret;

    if ((*android_global_javavm)->AttachCurrentThread(android_global_javavm, &ret, NULL) == JNI_OK) {
        pthread_setspecific(g_aenv_key, ret);
        return ret;
    }
    return NULL;
}

void mlt_android_jnienv_thrclean()
{
    (*android_global_javavm)->DetachCurrentThread(android_global_javavm);
    pthread_setspecific(g_aenv_key, NULL);
}

int mlt_android_sdk_version()
{
	static jint _ASDK_NUM = 0;
	if (_ASDK_NUM) return _ASDK_NUM;
	JNIEnv* env = mlt_android_get_jnienv();
	if (!env) return -1;

    jint sdk_int = (*env)->GetStaticIntField(env, gASDK.clazz, gASDK.jfid_SDK_INT);
    if (JNI_RethrowJVMException(env)) {
        return -1;
    }
    _ASDK_NUM = sdk_int;

    return sdk_int;

}

/**
#define SURFACE_CONTAINER_NAME "surface_container"

static void properties_clean_proxy(void* obj)
{
	mlt_properties props = (mlt_properties)obj;
	mlt_properties_dec_ref(props);
}

static void native_clean_proxy(void* obj)
{
	ANativeWindow* _obj = (ANativeWindow*)obj;
	ANativeWindow_release(_obj);
}

bool mlt_android_surface_regist(ANativeWindow* surface, const char* id)
{
	if (!surface || !id)return false;
	if ( g_aenv == NULL) {
		mlt_android_env_init();
		if (g_aenv == NULL) return false;
	}

	mlt_properties surface_container = (mlt_properties)mlt_properties_get_data
			(g_aenv,SURFACE_CONTAINER_NAME,NULL);
	if (surface_container == NULL) {
		surface_container = mlt_properties_new();
		mlt_properties_set_data(g_aenv, SURFACE_CONTAINER_NAME,
				surface_container, sizeof(mlt_properties),
				properties_clean_proxy,NULL);
	}

	void* previous = mlt_properties_get_data(surface_container, id, NULL);
	if ( previous ) {
		mlt_log_warning(NULL, "surface container duplicated for id:%s", id);
		return false;
	}
	ANativeWindow_acquire(surface);
	mlt_properties_set_data(surface_container, id, surface, sizeof(ANativeWindow*),
			native_clean_proxy, NULL);
	return true;
}

extern bool mlt_android_surface_detach(ANativeWindow* surface, const char* id)
{
	if (!surface || !id) return false;
	if (g_aenv == NULL) return true;
	mlt_properties surface_container = (mlt_properties)mlt_properties_get_data
			(g_aenv,SURFACE_CONTAINER_NAME,NULL);
	if (surface_container==NULL) return true;

	void* previous = mlt_properties_get_data(surface_container, id, NULL);
	if ( previous == NULL) return false;
	if ((ANativeWindow*)previous != surface) return false;

	mlt_properties_set_data(surface_container, id, NULL, sizeof(ANativeWindow*), NULL, NULL);
	return true;
}

ANativeWindow* mlt_android_surface_get(const char* id)
{
	if (!id) return NULL;
	if (g_aenv==NULL) return NULL;
	mlt_properties surface_container = (mlt_properties)mlt_properties_get_data
			(g_aenv,SURFACE_CONTAINER_NAME,NULL);
	if (surface_container==NULL) return NULL;

	void* previous = mlt_properties_get_data(surface_container, id, NULL);
	return (ANativeWindow*)previous;
}
**/
#ifdef DEBUG
ANativeWindow* g_testAWindow = NULL;
#endif

#endif

