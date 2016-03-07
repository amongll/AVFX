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
class Script;
class ScriptCallable
{
public:
	ScriptCallable(Script& caller, json_t* call_detail) throw(Exception);

	json_t* compile() throw(Exception);
	json_t* compile(vedit::ScriptType type) throw(Exception);
private:
	Script& script;
	void parse(json_t* call_detail) throw(Exception);
	string name;
	ScriptPropsPtr args;
};

NMSP_END(vedit)

#endif /* VEDITCALLABLE_H_ */
