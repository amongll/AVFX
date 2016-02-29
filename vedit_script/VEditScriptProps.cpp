/*
 * VEditScriptProps.cpp
 *
 *  Created on: 2016Äê2ÔÂ29ÈÕ
 *      Author: li.lei@youku.com
 */


#include "VEditScriptProps.h"

NMSP_BEGIN(vedit)

void ScriptProps::Property::expand_macro(const char* nm, const json_t* value)
		throw (Exception)
{
	MacroExpandable::expand_macro(nm, value);
	if (!MacroExpandable::finished() ) {
		return;
	}


}

void ScriptProps::Property::expand_scalar(const char* nm, const json_t* v)
		throw (Exception)
{
}

void ScriptProps::Property::expand_position(const char* nm, const int& frame_in,
		const int& frame_out, int frame_seq) throw (Exception)
{
}

void ScriptProps::expand_enum(const char* ename, const char* sname,
		const char* param_tag, const json_t* value) throw (Exception)
{
	EnumExpandable::expand_enum(ename,sname, param_tag, value);
	if ( !finished() ) {
		return;
	}

	json_t* after_enum_expand = EnumExpandable::expand_context;
	void* it = json_object_iter(after_enum_expand);
	do {
		const char* pnm = json_object_iter_key(it);
		json_t* v = json_object_iter_value(it);
		Property* pobj = new Property(*this, pnm, v);
		props.insert( make_pair(string(pnm), shared_ptr<Property>(pobj)) ) ;
		it = json_object_iter_next(after_enum_expand, it);
	} while ( it) ;
}

ScriptProps::ScriptProps(Script& script, json_t* detail,
		const char* enum_apply_tag) throw (Exception):
	EnumExpandable(script, detail, enum_apply_tag)
{
}

NMSP_END(vedit)


