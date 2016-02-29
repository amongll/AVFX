/*
 * VEditScriptEnums.cpp
 *
 *  Created on: 2016Äê2ÔÂ25ÈÕ
 *      Author: li.lei@youku.com
 */


#include "VEditScriptEnums.h"

NMSP_BEGIN(vedit)

ScriptEnums::ScriptEnums(Script& script):
	parent(script),
	defines(NULL)
{
	const json_t* t = script.get_enums_define();
	if (t && json_is_object(t) && json_object_size(t) ) {
		defines = json_deep_copy(t);
		try {
			parse_enums();

		} catch( const Exception &e) {
			json_decref(defines);
			defines = NULL;
			throw;
		}
	}
}

void ScriptEnums::parse_enums() throw (Exception)
{
	void* eit = json_object_iter(defines);
	const char* enm = NULL;
	json_t* se = NULL, *sse;
	do {
		enm = json_object_iter_key(eit);
		se = json_object_iter_value(eit);

		if ( is_valid_identifier(enm) == false ) {
			throw Exception(ErrorEnumDefineError, "enum name :%s invalid", enm);
		}

		if ( json_is_object(se) && json_object_size(se) ) {
			void* ii = json_object_iter(se);
			do {
				const char* snm = json_object_iter_key(ii);
				sse = json_object_iter_value(sse);

				if ( is_valid_identifier(enm) == false ) {
					throw Exception(ErrorEnumDefineError, "enum select %s->%s invalid", enm, snm);
				}

				JsonWrap* sobj = new JsonWrap(sse);
				enums[enm][snm].reset(sobj);

				ii = json_object_iter_next(se, ii);
			} while(ii);
		}

		eit = json_object_iter_next(defines, eit);
	} while(eit);
}

ScriptEnums::JsonWrap::JsonWrap(json_t* detail) throw (Exception) : js(NULL)
{
	if ( json_is_object(detail) ) {
		js = json_object();

		void* it = json_object_iter(detail);
		char* k;
		json_t* se;
		do {
			k = json_object_iter_key(it);
			se = json_object_iter_value(it);

			if ( json_is_object(se) || json_is_array(se) ) {
				continue;
			}

			json_object_set(js, k, se);
			it = json_object_iter_next(detail, it);
		} while(it);

	}
	else {
		throw Exception(ErrorEnumDefineError);
	}
}

ScriptEnums::~ScriptEnums() {
	if (defines) json_decref(defines);
}


NMSP_END(vedit)
