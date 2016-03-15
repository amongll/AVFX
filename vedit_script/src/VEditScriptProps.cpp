/*
 * VEditScriptProps.cpp
 *
 *  Created on: 2016��2��29��
 *      Author: li.lei@youku.com
 */


#include "VEditScriptProps.h"
#include "VEditScript.h"

NMSP_BEGIN(vedit)

ScriptProps::Property::Property(ScriptProps& p, const char* nm, json_t* detail)
		throw (Exception):
		Evaluable(),
		MacroExpandable(detail),
		parent(p)
{
	MacroExpandable::regist_self(parent.script);
	if ( !MacroExpandable::finished() ) {
		return;
	}

	Evaluable::parse(parent.script, MacroExpandable::expand_result);
}

ScriptProps::Property::~Property()
{
}

void ScriptProps::Property::expand_macro(const char* nm, const json_t* value)
		throw (Exception)
{
	MacroExpandable::expand_macro(nm, value);
	if ( !MacroExpandable::finished() ) {
		return;
	}

	Evaluable::parse(parent.script, MacroExpandable::expand_result);
}

void ScriptProps::Property::expand_scalar(const char* nm, const json_t* v)
		throw (Exception)
{
	if ( agent.get() ) {
		if ( agent->delegates.find(nm) != agent->delegates.end() ) {
			agent->expand_scalar(*this, nm, v);
			return;
		}
	}

	Evaluable::expand_scalar(nm, v);
}

void ScriptProps::Property::expand_position(const char* nm, const int& frame_in,
		const int& frame_out, int frame_seq) throw (Exception)
{
	if ( agent.get() ) {
		if ( agent->delegates.find(nm) != agent->delegates.end() ) {
			agent->expand_position(*this, nm, frame_in, frame_out, frame_seq);
			return;
		}
	}

	Evaluable::expand_position(nm, frame_in, frame_out, frame_seq);
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
	while ( it) {
		const char* pnm = json_object_iter_key(it);
		json_t* v = json_object_iter_value(it);
		Property* pobj = new Property(*this, pnm, v);
		props.insert( make_pair(string(pnm), shared_ptr<Property>(pobj)) ) ;
		it = json_object_iter_next(after_enum_expand, it);
	}
}

ScriptProps::Property& ScriptProps::get_property(const char* nm) throw (Exception)
{
	MapIter it =  props.find(nm);
	if ( it == props.end() ) {
		throw_error_v(ErrorImplError, "Property not found or script not enum expanded");
	}

	return *it->second.get();
}


ScriptProps::PropertyAgent::PropertyAgent(const set<string>& delgs):
	delegates(delgs)
{
}

ScriptProps::Property& ScriptProps::add_property(const char* nm, json_t* value)
		throw (Exception)
{
	if (json_is_object(value) || json_is_array(value) ) {
		throw_error_v(ErrorImplError, "Only literal value allowed for added property");
	}
	Property* _tmp  = new Property(*this, nm, value);
	if ( _tmp->MacroExpandable::finished() == false ) {
		throw_error_v(ErrorImplError, "Only literal value allowed for added property");
	}
	if ( _tmp->Evaluable::finished() == false ) {
		throw_error_v(ErrorImplError, "Only literal value allowed for added property");
	}

	MapIter it =  props.find(nm);
	if ( it != props.end() ) {
		script.unregist_evaluable(it->second.get());
	}

	props[nm].reset(_tmp);
	return *_tmp;
}

ScriptProps::ScriptProps(Script& script, json_t* detail,
		const char* enum_apply_tag) throw (Exception):
	EnumExpandable(script, detail, enum_apply_tag)
{
	if ( !finished() ) {
		return;
	}

	json_t* after_enum_expand = EnumExpandable::expand_context;
	void* it = json_object_iter(after_enum_expand);
	while ( it) {
		const char* pnm = json_object_iter_key(it);
		json_t* v = json_object_iter_value(it);
		Property* pobj = new Property(*this, pnm, v);
		props.insert( make_pair(string(pnm), shared_ptr<Property>(pobj)) ) ;
		it = json_object_iter_next(after_enum_expand, it);
	}
}

ScriptProps::ScriptProps(Script& script, const vector<string>& spec_props)
		throw (Exception):
	EnumExpandable(script, script.defines, spec_props)
{
	if ( !finished() ) {
		return;
	}

	json_t* after_enum_expand = EnumExpandable::expand_context;
	void* it = json_object_iter(after_enum_expand);
	while ( it) {
		const char* pnm = json_object_iter_key(it);
		json_t* v = json_object_iter_value(it);
		Property* pobj = new Property(*this, pnm, v);
		props[pnm].reset(pobj);
		//props.insert( make_pair(string(pnm), shared_ptr<Property>(pobj)) ) ;
		it = json_object_iter_next(after_enum_expand, it);
	}
}

json_t* ScriptProps::compile() throw (Exception)
{
	if (EnumExpandable::finished() == false) {
		throw_error_v(ErrorImplError, "properties enums resolved incompletely");
	}
	json_t* ret = json_object();
	try {
		MapIter it;
		for (it = props.begin(); it != props.end(); it++ ) {
			json_object_set_new(ret, it->first.c_str(), it->second->compile());
		}
	}
	catch(const Exception &e) {
		json_decref(ret);
		throw;
	}
	return ret;
}

NMSP_END(vedit)


