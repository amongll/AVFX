/*
 * VEditScript.cpp
 *
 *  Created on: 2016��2��24��
 *      Author: li.lei@youku.com
 */

#include "VEditScript.h"
#include "VEditVM.h"

NMSP_BEGIN(vedit)

void Script::call(json_t* args_value) throw (Exception)
{
	mlt_props.reset();
	type_spec_props.reset();
	selector_enum_presents.clear();
	macro_presents.clear();
	param_enum_presents.clear();
	param_evalue_presents.clear();

	if (args) json_decref(args);
	if (compiled) {
		json_decref(compiled);
		compiled = NULL;
	}

	if (args_value==NULL) {
		args = json_object();
	}
	else {
		args = json_incref(args_value);
	}
	json_t* t = args;

	json_t* se = json_object_get(defines,"props");
	parse_mlt_props(se);
	parse_specific();

	parse_filter_scriptcall();

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
			throw_error_v(ErrorScriptArgInvalid,"arg:%s should be enum selector",
				it->second->name);
		}
		else if ( strlen(json_string_value(arg)) == 0 ) {
			sel = it->second->default_selector;
		}
		else {
			sel = json_string_value(arg);
		}

		pair<EnumExpandableIter, EnumExpandableIter> ranges =
			param_enum_presents.equal_range(string(it->second->name));

		const json_t* enum_detail = get_selector(it->second->enum_name, sel);
		if ( !enum_detail && ranges.first != param_enum_presents.end() ) {
			throw_error_v(ErrorScriptArgInvalid,"arg:%s is invalid enum selector",
				it->second->name);
		}

		EnumExpandableIter eit ;
		for( eit = ranges.first; eit != ranges.second; eit ++) {
			eit->second->expand_enum(it->second->enum_name, sel,
				it->second->name, enum_detail);
		}

		param_enum_presents.erase(ranges.first, ranges.second);
	}

	applies_macros();

	for ( it = params->params.begin(); it != params->params.end(); it++ ) {
		if ( it->second->param_style == ScriptParams::EnumParam ) continue;
		if ( it->second->param_style == ScriptParams::PosParam ) continue;

		const json_t* arg = t ? json_object_get(t, it->second->name) : NULL;

		if ( it->second->param_style == ScriptParams::ScalarParam ) {
			if ( !arg ) {
				arg = it->second->default_scalar;
				//if ( !arg ) {
				//	throw_error_v(ErrorScriptArgInvalid,
				//		"arg:%s empty and has no default", it->second->name);
				//}
			}
			if (!arg || json_is_object(arg) || json_is_array(arg) ) {
				throw_error_v(ErrorScriptArgInvalid,"arg:%s should be scalar",
					it->second->name);
			}
			pair<EvaluableIter, EvaluableIter> ranges =
					param_evalue_presents.equal_range(string(it->second->name));

			if (!arg && ranges.first != param_evalue_presents.end() ) {
				throw_error_v(ErrorScriptArgInvalid,"arg:%s is needed",
					it->second->name);
			}

			EvaluableIter eit;
			for ( eit = ranges.first; eit != ranges.second; eit++ ) {
				eit->second->expand_scalar(eit->first.c_str(),arg);
			}
		}
		else if ( it->second->param_style == ScriptParams::DuraParam ) {
			int iarg = it->second->default_pos;
			if ( arg )  {
				if ( json_is_integer(arg)) {
					iarg = json_integer_value(arg);
				}
				else {
					throw_error_v(ErrorScriptArgInvalid,"arg:%s should be integer",
						it->second->name);
				}
			}

			if ( it->second->pos_type == ScriptParams::TimePos ) {
				iarg /= 40;
			}

			pair<EvaluableIter, EvaluableIter> ranges =
				param_evalue_presents.equal_range(string(it->second->name));

			JsonWrap iwrap(json_integer(iarg), 1);
			EvaluableIter eit;
			for ( eit = ranges.first; eit != ranges.second; eit++ ) {
				eit->second->expand_scalar(eit->first.c_str(),iwrap.h);
			}
		}
	}

	pre_judge();

	if( frame_out < 0 || frame_in < 0 || frame_in > frame_out) {
		throw_error_v(ErrorImplError, "position param can't be calced:%s ", it->second->name);
	}

	for ( it = params->params.begin(); it != params->params.end(); it++ ) {
		if ( it->second->param_style == ScriptParams::EnumParam ) continue;
		if ( it->second->param_style == ScriptParams::ScalarParam ) continue;
		if ( it->second->param_style == ScriptParams::DuraParam ) continue;

		const json_t* arg = t ? json_object_get(t, it->second->name) : NULL;

		if ( it->second->param_style == ScriptParams::PosParam ) {
			int iarg = it->second->default_pos;
			if ( arg )  {
				if ( json_is_integer(arg)) {
					iarg = json_integer_value(arg);
				}
				else {
					throw_error_v(ErrorScriptArgInvalid,"arg:%s should be integer",
						it->second->name);
				}
			}

			if ( it->first == "in" ) {
				iarg = 0;
			}
			else if ( it->first == "out" ) {
				iarg = frame_out - frame_in;
			}
			else if ( it->second->pos_type == ScriptParams::FramePos) {
				if ( iarg < 0 ) {
					if ( iarg  < (frame_in - frame_out - 1) )
						iarg = 0;
					else {
						iarg = (frame_out - frame_in) + iarg + 1;
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
				int totol_time_ms = (frame_out-frame_in+1)*40;
				if ( iarg < 0 ) {
					if ( iarg == -1 ) {
						iarg = frame_out - frame_in;
					}
					else {
						if ( iarg < -totol_time_ms) {
							iarg = 0;
						}
						else  {
							iarg = totol_time_ms + iarg;
						}
						iarg /= 40;
					}
				}
				else {
					if ( iarg > totol_time_ms ) {
						iarg = frame_out - frame_in;
					}
					else {
						iarg /= 40;
					}
				}
			}

			pair<EvaluableIter, EvaluableIter> ranges =
					param_evalue_presents.equal_range(string(it->second->name));
			EvaluableIter eit;
			for ( eit = ranges.first; eit != ranges.second; eit++ ) {
				if ( it->second->pos_relative == false) {
					eit->second->expand_position(eit->first.c_str(), iarg + frame_in);
				}
				else{
					eit->second->expand_position(eit->first.c_str(),frame_in, frame_out, iarg);
				}
			}
		}
	}
}

ScriptSerialized Script::get_mlt_serialize() throw (Exception)
{
	if (!compiled) {
		check_evaluables();
		compiled = serialize_mlt(0);
	}

	if (!compiled) {
		return make_pair<const char*, JsonWrap>(NULL, JsonWrap(NULL));
	}

	const char* uuid = json_string_value(json_object_get(compiled,"uuid"));
	return make_pair<const char*, JsonWrap>(uuid, JsonWrap(compiled, 0));
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
	throw_error(ErrorFeatureNotImpl);
}

void Script::add_macros(ScriptMacrosPtr macros) throw (Exception) {
	throw_error(ErrorFeatureNotImpl);
}

bool Script::add_enums(ScriptEnumsPtr enums) throw (Exception) {
	throw_error(ErrorFeatureNotImpl);
}

Script::~Script() {
	if (defines) json_decref(defines);
	if (args) json_decref(args);
}

void Script::regist_macro_usage(const char* macro, MacroExpandable* obj)
		throw (Exception)
{
	if ( macros->get_macro(macro) == NULL ) {
		throw_error_v(ErrorMacroNotFound, "macro:%s", macro);
	}

	pair<MacroExpandableIter, MacroExpandableIter>
		ranges = macro_presents.equal_range(macro);

	MacroExpandableIter it;
	for (it = ranges.first; it != ranges.second; it++) {
		if ( it->second == obj )
			return;
	}

	macro_presents.insert( make_pair(string(macro), obj) ) ;
}

void Script::regist_enum_selector_usage(const char* enmae, const char* sname,
		EnumExpandable* obj) throw (Exception) {

	if ( enums->get_selector(enmae, sname) == NULL) {
		throw_error_v(ErrorEnumSelectNotFound, "enum:%s:%s", enmae, sname);
	}

	string nm(enmae);
	nm.append(":").append(sname);

	pair<EnumExpandableIter, EnumExpandableIter> ranges =
			selector_enum_presents.equal_range(nm);
	EnumExpandableIter it ;
	for (it = ranges.first; it != ranges.second; it++ ) {
		if ( it->second == obj )
			return;
	}
	selector_enum_presents.insert( make_pair(nm, obj ) );
}

void Script::regist_enum_param_usage(const char* param, EnumExpandable* obj)
		throw (Exception)
{
	const ScriptParam* param_obj = params->get_param(param);
	if ( param_obj == NULL) {
		throw_error_v(ErrorParamNotFount, "param:%s", param);
	}

	if (param_obj->param_style != ScriptParams::EnumParam ) {
		throw_error_v(ErrorInvalidParam, "param:%s not enum", param);
	}

	pair<EnumExpandableIter,EnumExpandableIter> ranges =
			param_enum_presents.equal_range(param);
	EnumExpandableIter it;
	for (it = ranges.first; it != ranges.second; it++ ) {
		if (it->second == obj)
			return;
	}

	param_enum_presents.insert(make_pair( string(param), obj));
}

void Script::regist_scalar_param_usage(const char* param, Evaluable* obj)
		throw (Exception)
{
	const ScriptParam* param_obj = params->get_param(param);
	if ( param_obj == NULL) {
		throw_error_v(ErrorParamNotFount, "param:%s", param);
	}

	if (param_obj->param_style != ScriptParams::ScalarParam &&
			param_obj->param_style != ScriptParams::PosParam &&
			param_obj->param_style != ScriptParams::DuraParam) {
		throw_error_v(ErrorInvalidParam, "param:%s not scalar", param);
	}

	pair<EvaluableIter,EvaluableIter> ranges =
			param_evalue_presents.equal_range(param);
	EvaluableIter it ;
	for(it = ranges.first; it != ranges.second ; it++ ) {
		if ( it->second == obj )
			return;
	}

	param_evalue_presents.insert( make_pair(string(param), obj));
	all_pendings.insert(obj );
}

Script::Script(const json_t* detail) throw (Exception):
		proc_name(NULL),
		proc_type(NULL),
		desc(NULL),
		defines(NULL),
		frame_in(0),
		frame_out(-1),
		args(NULL),
		compiled(NULL)
{
	json_t* t = const_cast<json_t*>(detail);
	if ( !json_is_object(t) || json_object_size(t)==0 ) {
		throw_error(ErrorScriptFmtError);
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
		throw_error_v(ErrorScriptFmtError, "procname is invalid");
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
	//if (!detail)return;
	if ( detail && !json_is_object(detail) ) throw_error_v(ErrorScriptFmtError, "\"props\":{...} fmt needed");
	//if ( json_object_size(detail) == 0) return;

	ScriptProps* props = new ScriptProps(*this, detail);
	mlt_props.reset(props);
}

void Script::applies_selectors() throw (Exception)
{
	EnumExpandableIter it = selector_enum_presents.begin();

	for(; it != selector_enum_presents.end(); it++ ) {
		string enum_nm = it->first.substr(0, it->first.find(":"));
		string sel_nm = it->first.substr(it->first.find(":") + 1);
		const json_t* evalue = get_selector(enum_nm.c_str(), sel_nm.c_str());
		it->second->expand_enum(enum_nm.c_str(), sel_nm.c_str(), NULL, evalue);
	}
	selector_enum_presents.clear();
}

void Script::applies_macros() throw (Exception)
{
	MacroExpandableIter it = macro_presents.begin();
	for ( ; it != macro_presents.end(); it++) {
		const json_t* macrovalue = get_macro(it->first.c_str());
		it->second->expand_macro(it->first.c_str(), macrovalue);
	}
	macro_presents.clear();
}

json_t* Script::get_arg_value(const char* nm) throw (Exception)
{
	const ScriptParam* param = get_param_info(nm);
	json_t* arg = NULL;
	if (args) arg = json_object_get(args, nm);
	if (args == NULL || arg == NULL) {
		if ( param == NULL ) {
			return NULL;
		}
		else if (param->param_style == ScriptParams::PosParam ) {
			return json_integer(param->default_pos);
		}
		else if (param->param_style == ScriptParams::ScalarParam ) {
			if (param->default_scalar)
				return json_incref((json_t*)param->default_scalar);
			else
				return NULL;
		}
		else
			return NULL;
	}
	else {
		json_t* _a = json_object_get(args,nm);
		if ( param->param_style == ScriptParams::PosParam) {
			if ( !json_is_integer(_a))
				return NULL;
			else
				return json_incref(_a);
		}
		else if (param->param_style == ScriptParams::ScalarParam) {
			if ( json_is_object(_a) || json_is_array(_a))
				return NULL;
			else
				return json_incref(_a);
		}
	}
	return NULL;
}

void Script::set_frame_range(int in, int out) throw (Exception)
{
	assert( in >= 0 && out >= 0 && out >= in);
	frame_in = in;
	frame_out = out;
}

json_t* Script::serialize_mlt(int dummy)  throw (Exception)
{
	json_t* ret = json_object();

	json_object_set_new(ret, "proctype", json_string(proc_type));
	json_object_set_new(ret, "procname", json_string(proc_name));
	json_object_set_new(ret, "desc", json_string(desc));

	if ( type_spec_props.get()) {
		try {
			json_t* spec_props =  type_spec_props->compile();
			void* it = json_object_iter(spec_props);
			while ( it ) {
				const char* k = json_object_iter_key(it);
				json_t* se = json_object_iter_value(it);

				json_object_set(ret, k, se);

				it = json_object_iter_next(spec_props, it);
			}

			json_decref(spec_props);
		}
		catch(const Exception& e) {
			json_decref(ret);
			throw;
		}
	}

	if ( mlt_props.get()) {
		try {
			json_t* cpl = mlt_props->compile();
			json_object_set(ret,"props", cpl);
			json_decref(cpl);
		}
		catch(const Exception& e) {
			json_decref(ret);
			throw;
		}
	}

	try{
		json_t* filter_seris = filters_serialize();
		if (filter_seris)
			json_object_set_new(ret, "effects", filter_seris);
	}
	catch(const Exception& e){
		json_decref(ret);
		throw;
	}

	json_t* subdetail = compile();
	if (subdetail) {
		void* it = json_object_iter(subdetail);
		while(it) {
			const char* k = json_object_iter_key(it);
			json_t* se = json_object_iter_value(it);

			json_object_set(ret, k, se);

			it = json_object_iter_next(subdetail, it);
		}
		json_decref(subdetail);
	}

	json_t* uuid_se = json_object_get(ret, "uuid");
	if ( !uuid_se || !json_is_string(uuid_se) || !strlen(json_string_value(uuid_se)) ) {
		json_object_set_new(ret, "uuid", json_string(Vm::uuid().c_str()));
	}
	return ret;
}

void Script::parse_specific_props(const vector<string>& prop_nms)
		throw (Exception)
{
	vector<string> copy = prop_nms;
	copy.push_back("uuid");
	ScriptProps* props = new ScriptProps(*this, copy);
	type_spec_props.reset(props);
}

void Script::parse_filter_scriptcall() throw (Exception)
{
	json_t* filter_defines = json_object_get(defines,"effects");
	if ( !filter_defines )
		return;
	if ( !json_is_array(filter_defines)) {
		throw_error_v(ErrorScriptFmtError, "effects should be filter script calls");
	}
	if ( json_array_size(filter_defines) == 0) return;

	int asz = json_array_size(filter_defines);
	filters.resize(asz);
	for ( int i =0 ; i<asz;  i++ ) {
		json_t* call_detail = json_array_get(filter_defines, i);
		ScriptCallable* call = new ScriptCallable(*this, call_detail);
		FilterWrap& obj = filters[i];
		obj.call.reset(call);
	}
}

json_t* Script::filters_serialize() throw (Exception)
{
	json_t* ret = json_array();
	JsonWrap wrap(ret, 1);

	FilterIter it;
	for ( it = filters.begin(); it != filters.end(); it++ ) {
		it->call_result = it->call->compile(FILTER_SCRIPT);
		if ( it->call_result.first && it->call_result.second)
			json_array_append(wrap.h, it->call_result.second.h);
	}

	if ( json_array_size(ret) == 0)
		return 0;

	return json_incref(wrap.h);
}

void Script::check_evaluables()
{
	if (selector_enum_presents.size() != 0 ||
		param_enum_presents.size() != 0 ||
		macro_presents.size() !=0 ) {
		throw_error_v(ErrorScriptArgInvalid, "script not compiled completed.");
	}

	EvaluableCheckIter it = all_pendings.begin();
	bool all_evalued = true;
	for (; it != all_pendings.end(); it++ ) {
		if ( !(*it)->finished( ) ) {
			all_evalued = false;
		}
	}
	if (all_evalued == false) {
		throw_error_v(ErrorScriptArgInvalid, "script not compiled completed.");
	}
	all_pendings.clear();
}

NMSP_END(vedit)


