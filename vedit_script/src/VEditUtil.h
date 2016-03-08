/*
 * VEditUtil.h
 *
 *  Created on: 2016��3��2��
 *      Author: li.lei@youku.com
 */

#ifndef VEDITUTIL_H_
#define VEDITUTIL_H_

#include "VEditCommon.h"

NMSP_BEGIN(vedit)

struct JsonWrap
{
	JsonWrap(json_t* js = NULL):
		h(NULL)
	{
		if (js)
			h = json_incref(js);
	}

	JsonWrap(const JsonWrap& rh):
		h(NULL)
	{
		if(rh.h)
			h = json_incref((json_t*)rh.h);
	}

	JsonWrap& operator=(const JsonWrap& rh)
	{
		if (h) json_decref(h);
		if (rh.h) h = json_incref((json_t*)rh.h);
		return *this;
	}

	operator bool()
	{
		if (h) return true;
		return false;
	}

	~JsonWrap() {
		if (h) json_decref(h);
	}

	json_t* h;
};

struct Lock
{
	Lock(pthread_mutex_t* lk = NULL):
		impl(lk)
	{
		if(impl)pthread_mutex_lock(impl);
	}
	~Lock()
	{
		if(impl)pthread_mutex_unlock(impl);
	}

private:
	pthread_mutex_t* impl;
	Lock(const Lock&);
	Lock& operator=(const Lock&);
};

size_t get_absolute_path(const string& path, string& dest);

std::ostream& operator<<(std::ostream& os, mlt_properties props);

NMSP_END(vedit)


#endif /* VEDITUTIL_H_ */
