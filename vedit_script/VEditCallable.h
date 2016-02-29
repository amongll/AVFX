/*
 * VEditCallable.h
 *
 *  Created on: 2016��2��24��
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
