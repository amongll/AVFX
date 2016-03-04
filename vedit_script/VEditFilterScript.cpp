/*
 * VEditFilterScript.cpp
 *
 *  Created on: 2016年3月7日
 *      Author: li.lei@youku.com
 */

#include "VEditFilterScript.h"
#include "VEditVM.h"

NMSP_BEGIN(vedit)


FilterScript::~FilterScript()
{
}

json_t* FilterScript::compile() throw (Exception)
{
	return NULL;
}

void FilterScript::parse_specific() throw (Exception)
{
	vector<string> specs;
	specs.push_back("filter");
	parse_specific_props(specs);
}

void FilterScript::pre_judge() throw (Exception)
{
	const ScriptParam* inparam = get_param_info("in");
	const ScriptParam* outparam = get_param_info("out");
	const ScriptParam* resparam = get_param_info("resource");

	if ( !inparam || inparam->param_style != ScriptParams::PosParam
		|| inparam->pos_type != ScriptParams::FramePos ||
		!outparam || outparam->param_style != ScriptParams::PosParam
		|| outparam->pos_type != ScriptParams::FramePos) {
		throw Exception(ErrorScriptArgInvalid, "in/out position param is required for %s script",
			Vm::proc_type_names[FILTER_SCRIPT]);
	}

	if ( resparam && resparam->param_style != ScriptParams::ScalarParam ) {
		throw Exception(ErrorScriptArgInvalid, "resource scalar param is required to be scalar type"
				"if exists for %s script",
			Vm::proc_type_names[FILTER_SCRIPT]);
	}

	json_t* in = get_arg_value("in");
	json_t* out = get_arg_value("out");

	if ( !in || !json_is_integer(in) ) {
		throw Exception(ErrorScriptArgInvalid, "in arg is required for %s script",
			Vm::proc_type_names[FILTER_SCRIPT]);
	}

	if ( !out || !json_is_integer(out) ) {
		throw Exception(ErrorScriptArgInvalid, "out arg is required for %s script",
			Vm::proc_type_names[FILTER_SCRIPT]);
	}

	int inframe = json_integer_value(in), outframe = json_integer_value(out);

	if (inframe < 0 || outframe < 0) {
		throw Exception(ErrorScriptArgInvalid, "in/out arg is required to be positive integer"
			" for %s script", Vm::proc_type_names[FILTER_SCRIPT]);
	}
	if (inframe > outframe) {
		inframe ^= outframe ^= inframe ^= outframe;
	}
	set_frame_range(inframe,outframe);
	mlt_props->add_property("in", in);
	mlt_props->add_property("out", out);

	json_t* res_arg = get_arg_value("resource");
	if (res_arg && json_is_string(res_arg) && strlen(json_string_value(res_arg))) {
		type_spec_props->add_property("resource", res_arg);
	}
}

NMSP_END(vedit)
