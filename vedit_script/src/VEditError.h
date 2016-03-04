/*
 * VEditError.h
 *
 *  Created on: 2016-2-23
 *      Author: li.lei@youku.com
 */

#ifndef VEDITERROR_H_
#define VEDITERROR_H_

#include "VEditCommon.h"

NMSP_BEGIN(vedit)

enum ErrorCode
{
	Success,
	ErrorUnknown,
	ErrorScriptFmtError,
	ErrorFeatureNotImpl,
	ErrorInvalidParam,
	ErrorParamDefineError,
	ErrorParamNotFount,
	ErrorEnumDefineError,
	ErrorMacroDefineError,
	ErrorMacroNotFound,
	ErrorEnumTypeNotFound,
	ErrorEnumSelectNotFound,
	ErrorScriptArgInvalid,
	ErrorStreamFileInvalid,
	ErrorFileSysError,
	ErrorImplError
};

const char* error_desc(ErrorCode code);

NMSP_END(vedit)



#endif /* VEDITERROR_H_ */
