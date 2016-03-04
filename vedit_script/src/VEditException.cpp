/*
 * VEditException.cpp
 *
 *  Created on: 2016年3月7日
 *      Author: li.lei@youku.com
 */

#include "VEditException.h"
#include <cstdarg>

NMSP_BEGIN(vedit)

Exception::Exception(ErrorCode x):
	code(x),
	detail(error_desc(x))
{
}

Exception::Exception(ErrorCode x, const char* fmt, ...):
	code(x),
	detail(error_desc(x))
{
	char buf[2048];
	va_list vl;
	va_start(vl,fmt);
	vsnprintf(buf, sizeof(buf), fmt, vl);
	va_end(vl);
	detail += " ";
    detail += buf;
}

Exception::~Exception() throw () {
}

const char* Exception::what() const throw ()
{
	return detail.c_str();
}

NMSP_END(vedit)


