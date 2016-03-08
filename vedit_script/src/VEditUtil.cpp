/*
 * VEditUtil.cpp
 *
 *  Created on: 2016-3-2
 *      Author: li.lei@youku.com
 */

#include "VEditUtil.h"
#include <cstdlib>
#include <unistd.h>

static  size_t navi_rpath_2abs(const char* path, char *abs, size_t abs_sz)
{
	char* p;
	size_t left=1025;
	char tmp_path[1025];
	if (!path || strlen(path)==0)
		return 0;

	p = tmp_path;
	if ( path[0] != '/' ) {
		getcwd(tmp_path,sizeof(tmp_path));
		p += strlen(tmp_path);
		left -= strlen(tmp_path);
		if (left<1)
			return 0;

		if ( *(p-1) == '/' ) {
			p--;
			left++;
			*p = 0;
		}
	}
	else {
		*p++ = '/';
		left--;
	}

	char* dup = strdup(path);
	char* tk,*tk_ctx;
	tk = strtok_r(dup,"/",&tk_ctx);
	for (; tk ; tk=strtok_r(NULL,"/",&tk_ctx) ) {
		if ( strcmp(tk,".") == 0 )
			continue;
		else if ( strcmp(tk,"..") == 0 ) {
			char * r = p - 1;
			while( r!=tmp_path && *r != '/' ) {
				left++;
				r--;
			}

			p = r+1;
		}
		else {
			if ( p-1 != tmp_path && *(p-1)!='/' ) {
				*p++ = '/';
				left--;
			}

			if (left<1 || left < strlen(tk)+1 ) {
				free(dup);
				return 0;
			}

			memcpy(p, tk, strlen(tk));
			p += strlen(tk);
			left -= strlen(tk);
		}
	}

	*p = 0;
	free(dup);

	if (abs == NULL || abs_sz==0) {
		return p - tmp_path;
	}
	else
	{
		memcpy(abs, tmp_path, abs_sz);
		abs[abs_sz-1] = 0;
		return p - tmp_path;
	}
}

NMSP_BEGIN(vedit)

size_t get_absolute_path(const string& path, string& dest)
{
	dest.clear();
	char buf[1024] = {0};
	size_t ret = navi_rpath_2abs(path.c_str(), buf, sizeof(buf));
	if (ret > 0)
		dest = buf;
	return ret;
}

std::ostream& operator <<(std::ostream& os, mlt_properties props)
{
	int count = mlt_properties_count(props);
	os<<"{"<<endl;
	for ( int i=0; i<count; i++) {
		char buf[1024];
		const char* name = mlt_properties_get_name(props, i);
		snprintf(buf, sizeof(buf), "%s: %s\n", name, mlt_properties_get(props, name));
		os << buf ;
	}
	os<<"}"<<endl;
	return os;
}

NMSP_END(vedit)


