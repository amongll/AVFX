/*
 * VEditException.h
 *
 *  Created on: 2016��2��23��
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
	Exception(ErrorCode code);
	Exception(ErrorCode code,const char* fmt,...);
	virtual ~Exception() throw() {}
	const ErrorCode code;
	const std::string detail;

	const char* what()const throw();

};


NMSP_END(vedit)

#endif /* VEDITEXCEPTION_H_ */
