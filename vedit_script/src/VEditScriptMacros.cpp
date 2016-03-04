/*
 * VEditScriptMacros.cpp
 *
 *  Created on: 2016��2��25��
 *      Author: li.lei@youku.com
 */

#include "VEditScriptMacros.h"
#include "VEditScript.h"

NMSP_BEGIN(vedit)

ScriptMacros::~ScriptMacros()
{
	if (define_copy)json_decref(define_copy);
}

/*
ScriptMacros::JsonWrap::JsonWrap(json_t* detail) throw (Exception) :
	js(NULL)
{
	js = json_incref(detail);
}*/

ScriptMacros::ScriptMacros(Script& script) throw (Exception):
	parent(script),
	define_copy(NULL)
{
	const json_t* t = parent.get_macros_define();
	if ( t && json_is_object(t) && json_object_size(t) ) {
		define_copy = json_deep_copy(t);
		try {
			parse_macros();
		}catch( const Exception& e) {
			json_decref(define_copy);
			define_copy = NULL;
			throw;
		}
	}
}

void ScriptMacros::parse_macros() throw (Exception) {
	void* it = json_object_iter(define_copy);
	while(it) {
		const char* nm = json_object_iter_key(it);
		json_t* se = json_object_iter_value(it);

		if ( json_is_object(se) || json_is_array(se) )
			continue;

		if ( is_valid_identifier(nm) == false)
			throw Exception(ErrorMacroDefineError, "macro indentifier:%s invalid", nm);

		//JsonWrap* mobj = new JsonWrap(se);
		//macros[nm].reset(mobj);
		macros[nm] = se;
		it = json_object_iter_next(define_copy, it);
	}
}

NMSP_END(vedit)


