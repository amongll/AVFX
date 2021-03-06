/*
 * VEditCallable.cpp
 *
 *  Created on: 2016-3-1
 *      Author: li.lei@youku.com
 */

#include "VEditCallable.h"
#include "VEditScript.h"
#include "VEditVM.h"

NMSP_BEGIN(vedit)

ScriptCallable::ScriptCallable(Script& caller, json_t* call_detail)
	throw (Exception):
	script(caller)
{
	parse(call_detail);
}

ScriptSerialized ScriptCallable::compile() throw (Exception)
{
	json_t* call_args = NULL;
	ScriptSerialized call_get;
	try {
		call_args = args->compile();
		std::tr1::shared_ptr<Script> script = Vm::get_script(name.c_str());
		script->call(call_args);
		call_get = script->get_mlt_serialize();
		json_decref(call_args);
	}
	catch(const Exception& e) {
		if (call_args)json_decref(call_args);
		throw ;
	}
	return call_get;
}

ScriptSerialized ScriptCallable::compile(ScriptType type) throw (Exception)
{
	json_t* call_args = NULL;
	ScriptSerialized call_get;
	try {
		call_args = args->compile();
		call_get = Vm::call_script(name.c_str(), type, call_args);
		json_decref(call_args);
	}
	catch(const Exception& e) {
		if (call_args)json_decref(call_args);
		throw ;
	}
	return call_get;
}

void ScriptCallable::parse(json_t* call_detail) throw (Exception)
{
	if ( ! json_is_object(call_detail) || json_object_size(call_detail) != 1) {
		throw_error_v(ErrorScriptFmtError, "Not a callable");
	}

	void* it = json_object_iter(call_detail);
	const char* k = json_object_iter_key(it);

	if ( ! is_call(k, name) ) {
		throw_error_v(ErrorScriptFmtError, "Not a callable");
	}

	json_t* args_detail = json_object_iter_value(it);

	if ( ! json_is_object(args_detail) ) {
		throw_error_v(ErrorScriptFmtError, "Not a callable");
	}

	ScriptProps* props = new ScriptProps(script,args_detail, "$apply_params");
	args.reset(props);
}

NMSP_END(vedit)

