/*
 * mlt_android_env.c
 *
 *  Created on: 2016Äê1ÔÂ18ÈÕ
 *      Author: L-F000000-PC
 */

#include "mlt_android_env.h"
#include "mlt_factory.h"
#include <libgen.h>

#ifdef ANDROID

JavaVM *gJavaVm = NULL;

static int load_properties(AAssetManager* mgr, mlt_properties self, const char *name)
{
	// Convert filename string encoding.
	mlt_properties_set( self, "_mlt_properties_load", name );
	mlt_properties_from_utf8( self, "_mlt_properties_load", "__mlt_properties_load" );
	name = mlt_properties_get( self, "__mlt_properties_load" );

	// Open the file
	AAsset* file = AAssetManager_open(mgr, name, AASSET_MODE_STREAMING );
	if ( !file) {
		return -1;
	}

	int ret = 0;
	char temp[ 1024 ];
	char last[ 1024 ] = "";

	do {
		ret = AAsset_read(file, temp, sizeof(temp));
		if ( ret < 0 ) {
			break;
		}
		else if (ret > 0 ) {
			int x = strlen( temp ) - 1;
			if ( temp[x] == '\n' || temp[x] == '\r' )
				temp[x] = '\0';

			// Check if the line starts with a .
			if ( temp[ 0 ] == '.' )
			{
				char temp2[ 1024 ];
				sprintf( temp2, "%s%s", last, temp );
				strcpy( temp, temp2 );
			}

			else if ( strchr( temp, '=' ) )
			{
				strcpy( last, temp );
				*( strchr( last, '=' ) ) = '\0';
			}

			// Parse and set the property
			if ( strcmp( temp, "" ) && temp[ 0 ] != '#' )
				mlt_properties_parse( self, temp );
		}
	} while( ret > 0);

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
	snprintf(buf, sizeof(buf), "%s", file);
	char* dir = dirname(buf);
	return my_get_dir(dir);
}

static bool my_get_dir(const char* dir_name)
{
	char buf[1024];
	snprintf(buf, sizeof(buf), "%s", dir_name);
	char* dir = dirname(buf);
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
		if ( true == my_get_dir(dir) ) {
			mkdir(dir_name, S_IRWXU);
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
		goto failed;
	}

	int i, count = mlt_properties_count(assMd5s);
	for ( i=0 ; i<count; i++) {
		const char* name = mlt_properties_get_name(assMd5s,i);
		const char* value = mlt_properties_get_value(assMd5s, i);

		memset(path,0x00,sizeof(path));
		memset(real_path,0x00,sizeof(real_path));

		snprintf(path,sizeof(path), "%s/%s", files_path, name);
		snprintf(real_path,sizeof(real_path), "%s.%s", path, value);

		struct stat statbuf;
		if ( 0 == lstat(path, &statbuf) ) {
			if ( !S_ISLNK(statbuf.st_mode) ) {
				snprintf(err,err_size, "%s not symbol link", path);
				goto failed;
			}
			else {
				char check_real[1024];
				readlink(path, check_real,sizeof(check_real));
				if ( strcmp( check_real, real_path) == 0 && !stat(path,&statbuf) ) {
					continue;
				}
			}
		}
		else {
			if ( false == my_get_dir_for_file(real_path) ) {
				snprintf(err,err_size,"check dir for %s failed", real_path);
				return false;
			}
		}

		AAsset* asset = AAssetManager_open(mgr, name, O_RDONLY);
		int fd = open(real_path, O_CREAT|O_EXCL, S_IRWXU);
		if ( false == copy_asset_file(fd, asset) ) {
			if (asset)AAsset_close(asset);
			if (fd != -1)close(fd);
			goto failed;
		}
		AAsset_close(asset);
		close(fd);

		unlink(path);
		symlink(real_path, path);
	}

	if (assMd5s)mlt_properties_close(assMd5s);
	return true;
failed:
	if (assMd5s)mlt_properties_close(assMd5s);
	return false;
}

#endif

