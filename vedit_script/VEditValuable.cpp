/*
 * VEditValuable.cpp
 *
 *  Created on: 2016��2��29��
 *      Author: li.lei@youku.com
 */


#include "VEditValuable.h"
#include "VEditScriptParams.h"

NMSP_BEGIN(vedit)
void Evaluable::expand_scalar(const char* nm, const json_t* v) throw (Exception)
{
	assert(replace_type == EValueScalarReplace || replace_type == EValueStringCtxReplace);
	assert( !json_is_object(v) && !json_is_array(v));

	if (evalued)
		return;

	if (replace_type == EValueScalarReplace) {
		evalued = json_incref((json_t*)v);
		return;
	}
	else if (replace_type == EValueStringCtxReplace || replace_type == EValueStringCtxReplace) {
		char buf[50] = {0};
		const char* str = buf;
		json_t* t = const_cast<json_t*>(v);
		if (json_is_string(t) ) {
			str = json_string_value(t);
		}
		else if ( json_is_integer(t) ) {
			sprintf(buf, "%lld", json_integer_value(t));
		}
		else if ( json_is_real(t) ) {
			sprintf(buf, "%f", json_real_value(t));
		}
		else {
			sprintf(buf, json_is_true(t)?"true":"false");
		}

		pair<MapIter,MapIter> ranges = param_idxes.find(nm);
		MapIter it;
		for ( it = ranges.first; it != ranges.second; it++ ) {
			segments[it->second] = str;
		}
		param_idxes.erase(ranges.first, ranges.second);

		if ( param_idxes.size() == 0 ) {
			string tmp;
			vector<string>::iterator i;
			for ( i = segments.begin(); i!= segments.end();i++)
				tmp += *i;

			evalued = json_string(tmp.c_str());
			return;
		}
	}
}

void Evaluable::expand_position(const char* nm, const int& frame_in,
		const int& frame_out, int frame_seq) throw (Exception)
{
	assert(replace_type == EValuePositionReplace || replace_type == EValueStringCtxReplace);
	assert( frame_out >= 0 && frame_in >= 0 && frame_out >= frame_in && frame_seq >= 0);
	if (evalued) return;

	int len = frame_out - frame_in;
	if ( frame_seq >= len ) frame_seq = len - 1;
	if ( replace_type == EValuePositionReplace) {
		evalued = json_integer(frame_seq);
	}
	else if (replace_type == EValueStringCtxReplace || replace_type == EValueStringCtxReplace) {
		char buf[50] = {0};
		sprintf(buf, "%d", frame_seq);
		pair<MapIter,MapIter> ranges = param_idxes.find(nm);
		MapIter it;
		for ( it = ranges.first; it != ranges.second; it++ ) {
			segments[it->second] = buf;
		}
		param_idxes.erase(ranges.first, ranges.second);

		if ( param_idxes.size() == 0 ) {
			string tmp;
			vector<string>::iterator i;
			for ( i = segments.begin(); i!= segments.end();i++)
				tmp += *i;

			evalued = json_string(tmp.c_str());
			return;
		}
	}
}

void Evaluable::parse(Script& script, json_t* detail) throw (Exception)
{
	assert( !json_is_object(detail) && !json_is_array(detail) ) ;
	original = json_incref(detail);

	if ( !json_is_string(original) ) {
		evalued = json_incref(original);
		replace_type = EValueNoReplace;
		return;
	}

	parse_params(json_string_value(original), param_idxes, segments);

	if ( param_idxes.size() == 0) {
		evalued = json_incref(original);
		replace_type = EValueNoReplace;
		return;
	}

	if ( param_idxes.size() == 1 && segments.size() == 1 ) {
		const ScriptParam* param = script.get_param_info(segments[0].c_str());
		if ( param->param_style == ScriptParams::PosParam ) {
			replace_type = EValuePositionReplace;
		}
		else if (param->param_style == ScriptParams::ScalarParam ) {
			replace_type = EValueScalarReplace;
		}
		script.regist_scalar_param_usage(segments[0].c_str(), this);
		return;
	}
	else {
		replace_type =	EValueStringCtxReplace;
		MapIter it;
		for ( it = param_idxes.begin(); it != param_idxes.end(); it++ ) {
			script.regist_scalar_param_usage(it->first.c_str(), this);
			const ScriptParam* param = script.get_param_info(it->first.c_str());
			if (param->param_style == ScriptParams::PosParam) {
				replace_type = EValueStringCtxReplace2;
			}
		}
	}

}

Evaluable::Evaluable():
	replace_type(EValueNotParsed),
	evalued(NULL),
	original(NULL)
{
}

Evaluable::~Evaluable()
{
	if (evalued) json_decref(evalued);
	if (original) json_decref(original);
}

void Evaluable::replace_asis(const char* nm, const string& v) throw (Exception)
{
	if (replace_type == EValueScalarReplace) {
		evalued = json_string(v.c_str());
		return;
	}
	else if (replace_type == EValueStringCtxReplace || replace_type == EValueStringCtxReplace) {
		pair<MapIter,MapIter> ranges = param_idxes.find(nm);
		MapIter it;
		for ( it = ranges.first; it != ranges.second; it++ ) {
			segments[it->second] = v;
		}
		param_idxes.erase(ranges.first, ranges.second);

		if ( param_idxes.size() == 0 ) {
			string tmp;
			vector<string>::iterator i;
			for ( i = segments.begin(); i!= segments.end();i++)
				tmp += *i;

			evalued = json_string(tmp.c_str());
			return;
		}
	}
	else {
		throw Exception(ErrorImplError, "Evaluable::replace_asis replace_type not allowed");
	}
}

void Evaluable::replace_asis(const char* nm, int v) throw (Exception)
{
	if (replace_type == EValueScalarReplace || replace_type == EValuePositionReplace) {
		evalued = json_integer(v);
		return;
	}
	else if (replace_type == EValueStringCtxReplace || replace_type == EValueStringCtxReplace) {
		pair<MapIter,MapIter> ranges = param_idxes.find(nm);
		MapIter it;
		char buf[50];
		sprintf(buf, "%d", v);
		for ( it = ranges.first; it != ranges.second; it++ ) {
			segments[it->second] = buf;
		}
		param_idxes.erase(ranges.first, ranges.second);

		if ( param_idxes.size() == 0 ) {
			string tmp;
			vector<string>::iterator i;
			for ( i = segments.begin(); i!= segments.end();i++)
				tmp += *i;

			evalued = json_string(tmp.c_str());
			return;
		}
	}
	else {
		throw Exception(ErrorImplError, "Evaluable::replace_asis replace_type not allowed");
	}
}

void Evaluable::replace_asis(const char* nm, double v) throw (Exception)
{
	if (replace_type == EValueScalarReplace) {
		evalued = json_real(v);
		return;
	}
	else if (replace_type == EValueStringCtxReplace || replace_type == EValueStringCtxReplace) {
		pair<MapIter,MapIter> ranges = param_idxes.find(nm);
		MapIter it;
		char buf[50];
		sprintf(buf, "%f", v);
		for ( it = ranges.first; it != ranges.second; it++ ) {
			segments[it->second] = buf;
		}
		param_idxes.erase(ranges.first, ranges.second);

		if ( param_idxes.size() == 0 ) {
			string tmp;
			vector<string>::iterator i;
			for ( i = segments.begin(); i!= segments.end();i++)
				tmp += *i;

			evalued = json_string(tmp.c_str());
			return;
		}
	}
	else {
		throw Exception(ErrorImplError, "Evaluable::replace_asis replace_type not allowed");
	}
}

void Evaluable::replace_asis(const char* nm, bool v) throw (Exception)
{
	if (replace_type == EValueScalarReplace) {
		evalued = json_boolean(v);
		return;
	}
	else if (replace_type == EValueStringCtxReplace || replace_type == EValueStringCtxReplace) {
		pair<MapIter,MapIter> ranges = param_idxes.find(nm);
		MapIter it;
		char buf[50];
		sprintf(buf, v?"true":"false");
		for ( it = ranges.first; it != ranges.second; it++ ) {
			segments[it->second] = buf;
		}
		param_idxes.erase(ranges.first, ranges.second);

		if ( param_idxes.size() == 0 ) {
			string tmp;
			vector<string>::iterator i;
			for ( i = segments.begin(); i!= segments.end();i++)
				tmp += *i;

			evalued = json_string(tmp.c_str());
			return;
		}
	}
	else {
		throw Exception(ErrorImplError, "Evaluable::replace_asis replace_type not allowed");
	}
}

/**
void Evaluable::apply_params(Script& script, json_t* args) throw (Exception)
{
	assert(json_is_object(args));
	if ( evalued ) return;
	if ( json_object_size(args) == 0 ) return;

	if (replace_type == EValuePositionReplace || replace_type == EValueStringCtxReplace2) {
		throw Exception(ErrorImplError, "prev evaluable property can't contain position params");
	}

	void* it = json_object_iter(args);
	while(it) {
		const char* k = json_object_iter_key(it);
		json_t* v = json_object_iter_value(it);

		assert( !json_is_object(v) && !json_is_array(v));
		const ScriptParam* param = script.get_param_info(k);

		if (param->param_style == ScriptParams::ScalarParam ) {
			expand_scalar(k, v);
		}

		it = json_object_iter_next(args, it);
	}
}**/

NMSP_END(vedit)


