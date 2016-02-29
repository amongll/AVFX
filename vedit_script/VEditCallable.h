/*
 * VEditCallable.h
 *
 *  Created on: 2016Äê2ÔÂ24ÈÕ
 *      Author: li.lei@youku.com
 */

#ifndef VEDITCALLABLE_H_
#define VEDITCALLABLE_H_
#include "VEditCommon.h"
#include "VEditScriptProps.h"

NMSP_BEGIN(vedit)

class ScriptCallable
{
public:
	json_t* call();

private:

	ScriptPropsPtr params;
};

NMSP_END(vedit)


#endif /* VEDITCALLABLE_H_ */
