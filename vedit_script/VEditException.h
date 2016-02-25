/*
 * VEditException.h
 *
 *  Created on: 2016Äê2ÔÂ23ÈÕ
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
	const ErrorCode code;
	const std::string detail;

	const char* what()const throw();

};


NMSP_END(vedit)

#endif /* VEDITEXCEPTION_H_ */
