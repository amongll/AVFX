/*
 * VEditScriptParams.cpp
 *
 *  Created on: 2016-2-22
 *      Author: li.lei@youku.com
 */


#include "VEditScriptParams.h"
#include "VEditScript.h"

using namespace std;

vedit::ScriptParams::Param::Param(const char* nm, json_t* detail,
		const Script& script) throw (Exception):
	name(nm),
	desc(NULL),
	define_detail(NULL),
	param_style(UnknownParamStyle),
	pos_type(UnknownPosType),
	pos_relative(true),
	enum_name(NULL),
	default_pos(0),
	default_scalar(NULL),
	default_selector(NULL)
{
	//DECLARE_CONST_MEM_MODIFIER(modifier, default_scalar, const json_t**);
	//*modifier = json_incref(detail);
    define_detail = json_incref(detail);
	try {
		parse(script);
	}
	catch( const Exception &e) {
		json_decref(detail);
		throw;
	}
}

void vedit::ScriptParams::Param::parse(const Script& script) throw (Exception)
{
	json_t* se = NULL;

	se = json_object_get(define_detail, "desc");
	if (!se)
		se = json_object_get(define_detail, "description");

	if (se && json_is_string(se)) {
		DECLARE_CONST_MEM_MODIFIER(mod, desc, const char**);
		*mod = json_string_value(se);
	}

	ParamStyle style = ScalarParam;
	se = json_object_get(define_detail,"type");

	const char* type = "scalar";
	if ( se && json_is_string(se) ) {
		type = json_string_value(se);
		if ( strlen(type) == 0 )
			type = "scalar";
	}

	json_t* default_define = json_object_get(define_detail, "default");
	DECLARE_CONST_MEM_MODIFIER(pmodifier, param_style, ParamStyle*);
	if ( !strcasecmp(type,"scalar") ) {
		*pmodifier = ScalarParam;

		if (default_define) {
			if (json_is_object(default_define) || json_is_array(default_define)
					|| json_is_null(default_define) ) {
				throw_error_v(ErrorParamDefineError,"default is not scalar value for param:%s",
					name);
			}
			DECLARE_CONST_MEM_MODIFIER(xmod, default_scalar, const json_t**);
			*xmod = default_define;
		}
	}
	else if ( !strcasecmp(type, "pos_frame") || !strcasecmp(type,"position_frame")
			|| !strcasecmp(type, "frame_pos") || !strcasecmp(type,"frame_position") ) {

		*pmodifier = PosParam;
		DECLARE_CONST_MEM_MODIFIER(xmod, pos_type, ParamPosType*);
		*xmod = FramePos;

		if (default_define) {
			if (!json_is_integer(default_define)) throw_error_v(ErrorParamDefineError,
					"default is not position for param:%s", name);

			DECLARE_CONST_MEM_MODIFIER(imod,default_pos,int*);
			*imod = json_integer_value(default_define);
		}

		json_t* pos_relative_je = json_object_get(define_detail, "relative");
		if ( pos_relative_je && json_is_false(pos_relative_je)) {
			DECLARE_CONST_MEM_MODIFIER(xxmod, pos_relative, bool*);
			*xxmod  = false;
		}
	}
	else if ( !strcasecmp(type, "pos_time") || !strcasecmp(type,"position_time")
				|| !strcasecmp(type, "time_pos") || !strcasecmp(type,"time_position") ) {

		*pmodifier = PosParam;
		DECLARE_CONST_MEM_MODIFIER(xmod, pos_type, ParamPosType*);
		*xmod = TimePos;
		if (default_define) {
			if (!json_is_integer(default_define)) throw_error_v(ErrorParamDefineError,
				"default is not position for param:%s", name);

			DECLARE_CONST_MEM_MODIFIER(imod,default_pos,int*);
			*imod = json_integer_value(default_define);
		}

		json_t* pos_relative_je = json_object_get(define_detail, "relative");
		if ( pos_relative_je && json_is_false(pos_relative_je)) {
			DECLARE_CONST_MEM_MODIFIER(xxmod, pos_relative, bool*);
			*xxmod  = false;
		}
	}
	else if ( type[0] == '#' ) {
		*pmodifier = EnumParam;
		const char* enum_str= type + 1;

		if (strlen(enum_str) == 0) throw_error_v(ErrorParamDefineError,
				"enum type is invalid for param:%s", name);

		if (!script.has_enum(enum_str)) {
			throw_error_v(ErrorEnumTypeNotFound,
				"param:%s", name);
		}

		if ( default_define ) {
			if (!json_is_string(default_define)) throw_error_v(ErrorParamDefineError,
				"default is not invalid enum selector for param:%s", name);

			if ( ! script.get_selector(enum_str,json_string_value(default_define))) {
				throw_error_v(ErrorEnumSelectNotFound,
						"default selector:%s for param:%s",json_string_value(default_define), name);
			}

			DECLARE_CONST_MEM_MODIFIER(smod, default_selector, const char**);
			*smod = json_string_value(default_define);
		}

		DECLARE_CONST_MEM_MODIFIER(smod, enum_name, const char**);
		*smod = enum_str;
	}
	else {
		throw_error(ErrorParamDefineError);
	}
}

vedit::ScriptParams::Param::~Param()
{
	json_t* js  = const_cast<json_t*>(define_detail);
	json_decref(js);
}

vedit::ScriptParams::~ScriptParams()
{
	if (defines)json_decref(defines);
}

vedit::ScriptParams::ScriptParams(Script& script) throw(Exception):
	parent(script),
	defines(NULL)
{
	const json_t* param_define = parent.get_params_define();
	if (param_define) {
		defines = json_deep_copy(param_define);
		try {
			this->parse_param_defines();
		}
		catch( const Exception& e) {
			json_decref(defines);
			defines = NULL;
			throw;
		}
	}
}

void vedit::ScriptParams::parse_param_defines() throw (Exception)
{
	assert(defines);
	void* it = json_object_iter(defines);
	json_t* se;

	while(it) {
		const char* nm = json_object_iter_key(it);
		se = json_object_iter_value(it);

		if ( json_is_object(se) && json_object_size(se) > 0 ) {
			Param* param = new Param(nm, se, parent);
			params[nm] = ParamPtr(param);
		}

		it = json_object_iter_next(defines,it);
	}

}


