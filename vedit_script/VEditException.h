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
	const ErrorCode code;
	const std::string detail;

	virtual const char* what()const throw();
};


NMSP_END(vedit)

#endif /* VEDITEXCEPTION_H_ */
