/*
 * VEditEnumExpandable.cpp
 *
 *  Created on: 2016Äê2ÔÂ29ÈÕ
 *      Author: li.lei@youku.com
 */

#include "VEditEnumExpandable.h"

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
	do {
		const char* k = json_object_iter_key(ji);
		json_t* kv = json_object_iter_value(ji);
		if (json_is_object(kv) || json_is_array(kv)) {
			assert(0);
		}

		it = value_max_tag.find(k);
		if ( it == value_max_tag.end() || it->second < pripty ) {
			json_object_set(expand_context, k, kv);
			value_max_tag[k] = pripty;
		}

		ji = json_object_iter_next(jvs,ji);
	} while(ji);

	enum_ctx_tags.erase(tag);
}

EnumExpandable::EnumExpandable(Script& s, json_t* ctx, const char* oper_name) throw (Exception):
	script(s),
	expand_context(NULL)
{
	if ( !ctx || !json_is_object(ctx) || !oper_name || !strlen(oper_name) ) {
		throw Exception(ErrorImplError, "enum context impl problem %s:%d", __FILE__,__LINE__);
	}

	json_t* enums = json_object_get(ctx, oper_name);
	if ( ! json_is_array(enums) ) {
		throw Exception(ErrorImplError, "script enum context problem %s:%d", __FILE__,__LINE__);
	}

	if ( json_array_size(enums) == 0)
		return;

	parse(enums);

	expand_context = json_copy(ctx);
	json_object_del(expand_context, oper_name);

	void* it = json_object_iter(expand_context);
	do {
		const char* k = json_object_iter_key(it);
		json_t* v = json_object_iter_value(it);

		value_max_tag[k] = -1;

		it = json_object_iter_next(expand_context, it);
	} while(it);
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
		if ( is_selector_only(it->first.c_str()) ) {
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
			throw Exception(ErrorScriptFmtError,"enum apply format error");

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
			throw Exception(ErrorScriptFmtError, "enum apply format error");
		}
	}
}


NMSP_END(vedit)

