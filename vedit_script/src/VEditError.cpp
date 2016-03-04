/*
 * VEditError.cpp
 *
 *  Created on: 2016年3月7日
 *      Author: li.lei@youku.com
 */

#include "VEditError.h"

NMSP_BEGIN(vedit)

const char* error_desc(ErrorCode code)
{
	switch(code){
	case Success:
		return "<success> ";
	case ErrorUnknown:
		return "<unknown error>";
	case ErrorScriptFmtError:
		return "<format error>";
	case ErrorFeatureNotImpl:
		return "<feature not implemented>";
	case ErrorInvalidParam:
		return "<param invalid>";
	case ErrorParamDefineError:
		return "<param defination error>";
	case ErrorParamNotFount:
		return "<param not found>";
	case ErrorEnumDefineError:
		return "<enum defination error>";
	case ErrorMacroDefineError:
		return "<macro defination error>";
	case ErrorMacroNotFound:
		return "<macro not found>";
	case ErrorEnumTypeNotFound:
		return "<enum type not found>";
	case ErrorEnumSelectNotFound:
		return "<enum selector not found>";
	case ErrorScriptArgInvalid:
		return "<arg invalid>";
	case ErrorStreamFileInvalid:
		return "<stream media file invalid>";
	case ErrorFileSysError:
		return "<file/dir not found or unexpected>";
	case ErrorImplError:
		return "<Implementation problem>";
	default:
		return "";
	}
}


NMSP_END(vedit)

