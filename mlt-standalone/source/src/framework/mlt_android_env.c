/*
 * mlt_android_env.c
 *
 *  Created on: 2016Äê1ÔÂ18ÈÕ
 *      Author: L-F000000-PC
 */

#include "mlt_android_env.h"
#include "mlt_factory.h"
#include <libgen.h>
#include <errno.h>
#include "mlt_log.h"
#ifdef ANDROID

JavaVM *mlt_android_global_javavm = NULL;

typedef enum property_parse_status_E {
	status_start=0,
	status_key,
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
	int v_crlb;
	char* key;
	char* value;
	property_parse_status_e state;
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
		case status_equal:
			context->value = context->buf + context->cur;
			switch ( p ) {
			case '\r':
				context->v_crlb = 1;
				context->state = status_cr;
				context->buf[context->cur]='\0';
				context->cur++;
				return 1;
				break;
			case '\n':
				context->v_crlb = 1;
				context->state = status_lb;
				context->buf[context->cur]='\0';
				context->cur++;
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
				return 1;
				break;
			case '\n':
				context->v_crlb = 1;
				context->state = status_lb;
				context->buf[context->cur]='\0';
				context->cur++;
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
					context->state = status_value;
					memmove(context->buf + context->cur, context->buf + context->cur + 1, context->end - context->cur - 1);
					context->end--;
					continue;
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

#endif

