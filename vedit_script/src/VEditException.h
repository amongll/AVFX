/*
 * VEditException.h
 *
 *  Created on: 2016-2-23
 *      Author: li.lei@youku.com
 */

#ifndef VEDITEXCEPTION_H_
#define VEDITEXCEPTION_H_

#include "VEditCommon.h"
#include <exception>
#include "VEditError.h"

NMSP_BEGIN(vedit)
class Exception : public std::exception
{
public:
	Exception(ErrorCode code = ErrorUnknown);
	Exception(ErrorCode code,const char* fmt,...);
	Exception(const Exception& r):
		code(r.code),
		detail(r.detail)
	{}

	Exception& operator=(const Exception& r) {
		code = r.code;
		detail = r.detail;
		return *this;
	}
	virtual ~Exception() throw() ;
	ErrorCode code;
	std::string detail;

	virtual const char* what()const throw();
};


NMSP_END(vedit)

#define throw_error(code) do{\
	throw Exception(code);\
}while(0)

#define throw_error_v(code, fmt, detail...) do{\
	char fmtbuf[512];\
	char buf[1024];\
	snprintf(fmtbuf, sizeof(fmtbuf),"%s at<%%s:%%d %%s>", fmt);\
	snprintf(buf,sizeof(buf), fmtbuf, ##detail, __FILE__,__LINE__,__PRETTY_FUNCTION__);\
	throw Exception(code, "%s", buf);\
}while(0)

#endif /* VEDITEXCEPTION_H_ */
