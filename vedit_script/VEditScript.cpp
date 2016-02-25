/*
 * VEditScript.cpp
 *
 *  Created on: 2016Äê2ÔÂ24ÈÕ
 *      Author: li.lei@youku.com
 */

#include "VEditScript.h"


NMSP_BEGIN(vedit)

void Script::applie_params(const json_t* param_values) throw (Exception) {
}

void Script::applie_properties(const mlt_properties* properties)
		throw (Exception) {
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

void Script::regist_mlt_property_usage(const char* property, Evaluable* obj)
		throw (Exception)
{
	pair<EvaluableIter,EvaluableIter> ranges =
			mlt_evalue_presents.equal_range(property);
	EvaluableIter it ;
	for(it = ranges.first; it != ranges.second ; it++ ) {
		if ( it->second.get() == obj )
			return;
	}

	mlt_evalue_presents.insert( make_pair(string(property), shared_ptr<Evaluable>(obj)));
	all_pendings.insert( make_pair(obj, shared_ptr<Evaluable>(obj)) );
}

NMSP_END(vedit)



