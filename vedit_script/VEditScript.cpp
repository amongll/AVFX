/*
 * VEditScript.cpp
 *
 *  Created on: 2016Äê2ÔÂ24ÈÕ
 *      Author: li.lei@youku.com
 */

#include "VEditScript.h"


NMSP_BEGIN(vedit)

void Script::applie_params(const json_t* args_value) throw (Exception)
{
	mlt_props.reset();
	type_spec_props.reset();
	json_decref(compiled);
	selector_enum_presents.clear();
	macro_presents.clear();
	param_enum_presents.clear();
	param_evalue_presents.clear();

	args = const_cast<json_t*>(args_value);
	json_t* t = args;

	json_t* se = json_object_get(defines,"props");
	parse_mlt_props(se);
	parse_specific();

	applies_selectors();

	ScriptParams::MapCIter it = params->params.begin();
	for ( ; it != params->params.end(); it++  ) {
		if ( it->second->param_style != ScriptParams::EnumParam ) continue;

		const char* sel = NULL;
		json_t* arg = t ? json_object_get(t, it->second->name) : NULL;
		if ( ! arg ) {
			sel = it->second->default_selector;
		}
		else if ( !json_is_string(arg) ) {
			throw Exception(ErrorScriptArgInvalid,"arg:%s should be enum selector",
				it->second->name);
		}
		else if ( strlen(json_string_value(arg)) == 0 ) {
			sel = it->second->default_selector;
		}
		else {
			sel = json_string_value(arg);
		}

		pair<EnumExpandableIter, EnumExpandableIter> ranges =
			param_enum_presents.find(it->second->name);

		const json_t* enum_detail = get_selector(it->second->enum_name, sel);
		if ( !enum_detail && ranges.first == param_enum_presents.end() ) {
			throw Exception(ErrorScriptArgInvalid,"arg:%s is invalid enum selector",
				it->second->name);
		}

		EnumExpandableIter eit ;
		for( eit = ranges.first; eit != ranges.second; eit ++) {
			eit->second->expand_enum(it->second->enum_name, sel,
				it->second->name, enum_detail);
		}
	}

	applies_macros();

	args = NULL;

	for ( it = params->params.begin(); it != params->params.end(); it++ ) {
		if ( it->second->param_style == ScriptParams::EnumParam ) continue;

		const json_t* arg = t ? json_object_get(t, it->second->name) : NULL;

		if ( it->second->param_style == ScriptParams::PosParam ) {
			int iarg = it->second->default_pos;
			if ( arg )  {
				if ( json_is_integer(arg)) {
					iarg = json_integer_value(arg);
				}
				else {
					throw Exception(ErrorScriptArgInvalid,"arg:%s should be integer",
						it->second->name);
				}
			}

			if( frame_out < 0 || frame_in < 0 || frame_in > frame_out) {
				throw Exception(ErrorImplError, "position param can't be calced:%s ", it->second->name);
			}

			if ( it->second->pos_type == ScriptParams::PerctPos ) {
				if ( iarg > 100) iarg = 100;
				if ( iarg < -100) iarg = -100;

				if ( iarg > 0) {
					iarg = (frame_out-frame_in) * (double)iarg/100;
				}
				else if ( iarg < 0 ) {
					iarg = (frame_out - frame_in) * (double)(100 + iarg)/100;
				}
			}
			else if ( it->second->pos_type == ScriptParams::FramePos) {
				if ( iarg < 0 ) {
					if ( iarg  < (frame_in - frame_out) )
						iarg = 0;
					else {
						iarg = (frame_out - frame_in) + iarg;
					}
				}
				else if ( iarg > (frame_out - frame_in) ) {
					iarg = frame_out - frame_in;
				}
				else {
					;//
				}

			}
			else if ( it->second->pos_type == ScriptParams::TimePos ) {
				iarg = (double)(iarg)/1000 * 25; // 25fps
				if ( iarg < 0 ) {
					if ( iarg  < (frame_in - frame_out) )
						iarg = 0;
					else {
						iarg = (frame_out - frame_in) + iarg;
					}
				}
				else if ( iarg > (frame_out - frame_in) ) {
					iarg = frame_out - frame_in;
				}
				else {
					;//
				}
			}

			pair<EvaluableIter, EvaluableIter> ranges =
					param_evalue_presents.find(it->second->name);
			EvaluableIter eit;
			for ( eit = ranges.first; eit != ranges.second; eit++ ) {
				eit->second->expand_position(eit->first.c_str(),frame_in, frame_out, iarg);
			}
		}
		else if ( it->second->param_style == ScriptParams::ScalarParam ) {
			if ( !arg ) {
				arg = it->second->default_scalar;
			}
			else if ( json_is_object(arg) || json_is_array(arg) ) {
				throw Exception(ErrorScriptArgInvalid,"arg:%s should be scalar",
					it->second->name);
			}
			else {
				pair<EvaluableIter, EvaluableIter> ranges =
						param_evalue_presents.find(it->second->name);

				if (!arg && ranges.first == param_evalue_presents.end() ) {
					throw Exception(ErrorScriptArgInvalid,"arg:%s is needed",
						it->second->name);
				}

				EvaluableIter eit;
				for ( eit = ranges.first; eit != ranges.second; eit++ ) {
					eit->second->expand_scalar(eit->first.c_str(),arg);
				}
			}
		}
	}

	check_evaluables();
}

const json_t* Script::get_params_define() const
{
	json_t* sub = json_object_get(defines,"params");
	if (sub == NULL) return NULL;
	if (!json_is_object(sub) || json_object_size(sub)==0) {
		return NULL;
	}
	return sub;
}

const json_t* Script::get_macros_define() const {
	json_t* sub = json_object_get(defines,"macros");
	if (sub == NULL) return NULL;
	if (!json_is_object(sub) || json_object_size(sub)==0) {
		return NULL;
	}
	return sub;
}

const json_t* Script::get_enums_define() const {
	json_t* sub = json_object_get(defines,"enums");
	if (sub == NULL) return NULL;
	if (!json_is_object(sub) || json_object_size(sub)==0) {
		return NULL;
	}
	return sub;
}

void Script::add_params(ScriptParamsPtr params) throw (Exception) {
	throw Exception(ErrorFeatureNotImpl);
}

void Script::add_macros(ScriptMacrosPtr macros) throw (Exception) {
	throw Exception(ErrorFeatureNotImpl);
}

bool Script::add_enums(ScriptEnumsPtr enums) throw (Exception) {
	throw Exception(ErrorFeatureNotImpl);
}

Script::~Script() {
	if (defines) json_decref(defines);
}

void Script::regist_macro_usage(const char* macro, MacroExpandable* obj)
		throw (Exception)
{
	if ( macros->get_macro(macro) == NULL ) {
		throw Exception(ErrorMacroNotFound, "macro:%s", macro);
	}

	pair<MacroExpandableIter, MacroExpandableIter>
		ranges = macro_presents.equal_range(macro);

	MacroExpandableIter it;
	for (it = ranges.first; it != ranges.second; it++) {
		if ( it->second.get() == obj )
			return;
	}

	macro_presents.insert( make_pair(string(macro), shared_ptr<MacroExpandable>(obj)) ) ;
}

void Script::regist_enum_selector_usage(const char* enmae, const char* sname,
		EnumExpandable* obj) throw (Exception) {

	if ( enums->get_selector(enmae, sname) == NULL) {
		throw Exception(ErrorEnumSelectNotFound, "enum:%s:%s", enmae, sname);
	}

	string nm(enmae);
	nm.append(":").append(sname);

	pair<EnumExpandableIter, EnumExpandableIter> ranges =
			selector_enum_presents.equal_range(nm);
	EnumExpandableIter it ;
	for (it = ranges.first; it != ranges.second; it++ ) {
		if ( it->second.get() == obj )
			return;
	}

	selector_enum_presents.insert( make_pair(nm, shared_ptr<EnumExpandable>(obj) ) );
}

void Script::regist_enum_param_usage(const char* param, EnumExpandable* obj)
		throw (Exception)
{
	const ScriptParam* param_obj = params->get_param(param);
	if ( param_obj == NULL) {
		throw Exception(ErrorParamNotFount, "param:%s", param);
	}

	if (param_obj->param_style != ScriptParams::EnumParam ) {
		throw Exception(ErrorInvalidParam, "param:%s not enum", param);
	}

	pair<EnumExpandableIter,EnumExpandableIter> ranges =
			param_enum_presents.equal_range(param);
	EnumExpandableIter it;
	for (it = ranges.first; it != ranges.second; it++ ) {
		if (it->second.get() == obj)
			return;
	}

	param_enum_presents.insert(make_pair( string(param), shared_ptr<EnumExpandable>(obj)));
}

void Script::regist_scalar_param_usage(const char* param, Evaluable* obj)
		throw (Exception)
{
	const ScriptParam* param_obj = params->get_param(param);
	if ( param_obj == NULL) {
		throw Exception(ErrorParamNotFount, "param:%s", param);
	}

	if (param_obj->param_style != ScriptParams::ScalarParam ) {
		throw Exception(ErrorInvalidParam, "param:%s not scalar", param);
	}

	pair<EvaluableIter,EvaluableIter> ranges =
			param_evalue_presents.equal_range(param);
	EvaluableIter it ;
	for(it = ranges.first; it != ranges.second ; it++ ) {
		if ( it->second.get() == obj )
			return;
	}

	param_evalue_presents.insert( make_pair(string(param), shared_ptr<Evaluable>(obj)));
	all_pendings.insert( make_pair(obj, shared_ptr<Evaluable>(obj)) );
}

Script::Script(const json_t* detail) throw (Exception):
		proc_name(NULL),
		proc_type(NULL),
		desc(NULL),
		defines(NULL),
		compiled(NULL),
		frame_in(0),
		frame_out(-1),
		args(NULL)

{
	json_t* t = const_cast<json_t*>(detail);
	if ( !json_is_object(t) || json_object_size(t)==0 ) {
		throw Exception(ErrorScriptFmtError);
	}

	defines = json_deep_copy(t);
	try {
		parse_impl();
	}
	catch(const Exception& e) {
		json_decref(defines);
		defines = NULL;
		throw;
	}
}

void Script::parse_impl() throw (Exception)
{
	json_t* se = NULL;

	se = json_object_get(defines, "procname");
	if ( !se || !json_is_string(se) || !strlen(json_string_value(se))) {
		throw Exception(ErrorScriptFmtError, "procname is invalid");
	}

	DECLARE_CONST_MEM_MODIFIER(smod, proc_name, const char**);
	*smod = json_string_value(se);

	se = json_object_get(defines, "proctype");
	assert( se && json_is_string(se) &&  strlen(json_string_value(se)));
	USE_CONST_MEM_MODIFIER(smod, proc_type, const char**);
	*smod = json_string_value(se);

	se = json_object_get(defines, "desc");
	if ( se && json_is_string(se)  ) {
		USE_CONST_MEM_MODIFIER(smod, desc, const char**);
		*smod = json_string_value(se);
	}

	ScriptEnums* enums = new ScriptEnums(*this);
	this->enums.reset(enums);

	ScriptMacros* macros = new ScriptMacros(*this);
	this->macros.reset(macros);

	ScriptParams* params = new ScriptParams(*this);
	this->params.reset(params);
}

void Script::parse_mlt_props(json_t* detail) throw (Exception)
{
	if (!detail)return;
	if ( !json_is_object(detail) ) throw Exception(ErrorScriptFmtError, "\"props\":{...} fmt needed");
	if ( json_object_size(detail) == 0) return;

	ScriptProps* props = new ScriptProps(*this, detail);
	mlt_props.reset(props);
}

void Script::applies_selectors() throw (Exception)
{
	EnumExpandableIter it;
	for ( it = selector_enum_presents.begin(); it != selector_enum_presents.end(); it++ ) {
		string enum_nm = it->first.substr(0, it->first.find(":"));
		string sel_nm = it->first.substr(it->first.find(":") + 1);
		const json_t* evalue = get_selector(enum_nm.c_str(), sel_nm.c_str());
		it->second->expand_enum(enum_nm.c_str(), sel_nm.c_str(), NULL, evalue);
	}
}

void Script::applies_macros() throw (Exception)
{
	MacroExpandableIter it;
	for ( it = macro_presents.begin(); it != macro_presents.end(); it++ ) {
		const json_t* macrovalue = get_macro(it->first.c_str());
		it->second->expand_macro(it->first.c_str(), macrovalue);
	}
}

void Script::check_evaluables()
{
	EvaluableCheckIter it = all_pendings.begin(), tit;
	while ( it != all_pendings.end() ) {
		if ( it->first->is_done( ) ) {
			tit = it++;
			all_pendings.erase(tit);
		}
	}
	if (all_pendings.size() > 0) {
		throw Exception(ErrorScriptArgInvalid, "script not compiled completed.");
	}
}

NMSP_END(vedit)
