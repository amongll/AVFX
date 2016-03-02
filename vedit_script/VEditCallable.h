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
#include "VEditVm.h"

NMSP_BEGIN(vedit)
class Script;
class ScriptCallable
{
public:
	ScriptCallable(Script& caller, json_t* call_detail) throw(Exception);

	json_t* compile() throw(Exception);
	json_t* compile(Vm::ScriptType type) throw(Exception);
private:
	Script& script;
	void parse(json_t* call_detail) throw(Exception);
	string name;
	ScriptPropsPtr args;
};

NMSP_END(vedit)

#endif /* VEDITCALLABLE_H_ */
