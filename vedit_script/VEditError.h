/*
 * VEditError.h
 *
 *  Created on: 2016Äê2ÔÂ23ÈÕ
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
	ErrorImplError
};

const char* error_desc(ErrorCode code);

NMSP_END(vedit)



#endif /* VEDITERROR_H_ */
