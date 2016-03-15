/*
 * VEditEnumExpandable.cpp
 *
 *  Created on: 2016��2��29��
 *      Author: li.lei@youku.com
 */

#include "VEditEnumExpandable.h"
#include "VEditScript.h"

NMSP_BEGIN(vedit)

void EnumExpandable::expand_enum(const char* ename, const char* sname,
		const char* param_tag, const json_t* value) throw (Exception)
{
	assert(json_is_object(value));

	string tag;
	if (param_tag) {
		tag = param_tag;
	}
	else {
		tag = string(ename) + ":" + sname;
	}

	hash_map<string,int>::iterator it = enum_ctx_tags.find(tag);
	assert( it != enum_ctx_tags.end());

	int pripty = it->second;

	enum_ctx_tags[tag] = -1;

	json_t* jvs = const_cast<json_t*>(value);
	void *ji = json_object_iter(jvs);
	while(ji) {
		const char* k = json_object_iter_key(ji);
		json_t* kv = json_object_iter_value(ji);
		assert(!json_is_object(kv) && !json_is_array(kv));

		it = value_max_tag.find(k);
		if ( it == value_max_tag.end() || it->second < pripty ) {
			json_object_set(expand_context, k, kv);
			value_max_tag[k] = pripty;
		}

		ji = json_object_iter_next(jvs,ji);
	}

	enum_ctx_tags.erase(tag);
}

EnumExpandable::EnumExpandable(Script& s, json_t* ctx, const char* oper_name) throw (Exception):
	script(s),
	expand_context(NULL)
{
	if ( (ctx && !json_is_object(ctx)) || !oper_name || !strlen(oper_name) ) {
		throw_error_v(ErrorImplError, "enum context impl problem %s:%d", __FILE__,__LINE__);
	}

	if ( ctx == NULL || json_object_size(ctx) == 0 )
		return;

	json_t* enums = json_object_get(ctx, oper_name);
	if ( enums && !json_is_array(enums) ) {
		throw_error_v(ErrorImplError, "script enum context problem %s:%d", __FILE__,__LINE__);
	}

	if ( enums ) {
		if ( json_array_size(enums) == 0)
			return;

		parse(enums);

		expand_context = json_copy(ctx);
		json_object_del(expand_context, oper_name);

		void* it = json_object_iter(expand_context);
		while( it ) {
			const char* k = json_object_iter_key(it);
			json_t* v = json_object_iter_value(it);

			value_max_tag[k] = -1;

			it = json_object_iter_next(expand_context, it);
		}
	}
	else {
		expand_context = json_copy(ctx);
	}
	register_self();
}

EnumExpandable::~EnumExpandable()
{
	if(expand_context)
		json_decref(expand_context);
}

void EnumExpandable::register_self() throw (Exception)
{
	hash_map<string, int>::iterator it ;
	for ( it = enum_ctx_tags.begin(); it != enum_ctx_tags.end(); it++ ) {
		//if ( is_selector_only(it->first.c_str()) ) {
		if ( it->first.find(':') != std::string::npos) {
			string enm,snm;
			enm = it->first.substr(0,it->first.find(':'));
			snm = it->first.substr(it->first.find(':')+1);

			script.regist_enum_selector_usage(enm.c_str(),snm.c_str(),this);
		}
		else {
			script.regist_enum_param_usage(it->first.c_str(), this);
		}
	}
}

void EnumExpandable::parse(json_t* values) throw (Exception)
{
	int i;
	for ( i=0; i<json_array_size(values); i++ ) {
		json_t* ae = json_array_get(values, i);
		if ( ! json_is_string(ae) )
			throw_error_v(ErrorScriptFmtError,"enum apply format error");

		const char* v = json_string_value(ae);

		if ( is_selector_only(v) ) {
			string enm,snm;
			parse_selector(v,enm,snm);

			string etag = enm + ":" + snm;
			enum_ctx_tags[etag] = i;
		}
		else if ( is_param_only(v) ) {
			vector<string> _ps;
			parse_params(v, _ps);

			enum_ctx_tags[_ps[0]] = i;
		}
		else {
			throw_error_v(ErrorScriptFmtError, "enum apply format error");
		}
	}
}

EnumExpandable::EnumExpandable(Script& s, json_t* ctx,
		const vector<string>& props) throw (Exception):
	script(s),
	expand_context(NULL)
{
	if ( !json_is_object(ctx) ) {
		throw_error_v(ErrorImplError, "Some Script sub class specific fmt error");
	}
	vector<string>::const_iterator it;
	expand_context = json_object();
	for (it = props.begin(); it!=props.end(); it++) {
		json_t* se = json_object_get(ctx, it->c_str());
		if ( se ) {
			if ( json_is_object(se) || json_is_array(se) ) {
				throw_error_v(ErrorImplError, "Some Script sub class specific fmt error");
			}
			else {
				json_object_set(expand_context, it->c_str(), se);
			}
		}
	}
}


NMSP_END(vedit)


