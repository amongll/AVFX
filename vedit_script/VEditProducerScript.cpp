/*
 * VEditProducerScript.cpp
 *
 *  Created on: 2016年3月3日
 *      Author: li.lei@youku.com
 */

#include "VEditProducerScript.h"
#include "VEditVM.h"

NMSP_BEGIN(vedit)


void SingleResourceScript::parse_specific() throw (Exception)
{
	/**
	vector<string> spec_prop_nms;
	spec_prop_nms.push_back("resource");
	parse_specific_props(spec_prop_nms);*/
}

VideoScript::~VideoScript()
{
}

json_t* VideoScript::compile() throw (Exception)
{
	mlt_producer prod = Vm::get_stream_resource(path);
	//todo: format info

	return NULL;
}

AudioScript::~AudioScript()
{
}

json_t* AudioScript::compile() throw (Exception)
{
}

ImageScript::~ImageScript()
{
}

json_t* ImageScript::compile() throw (Exception)
{
}

GifScript::~GifScript()
{
}

json_t* GifScript::compile() throw (Exception)
{
}

void VideoScript::pre_judge() throw (Exception)
{
	const ScriptParam* inparam = get_param_info("in");
	const ScriptParam* outparam = get_param_info("out");
	const ScriptParam* resparam = get_param_info("resource");

	if ( !inparam || inparam->param_style != ScriptParams::PosParam ||
		!outparam || outparam->param_style != ScriptParams::PosParam) {
		throw Exception(ErrorScriptArgInvalid, "in/out position param is required for %s script",
			Vm::proc_type_names[VIDEO_RESOURCE_SCRIPT]);
	}

	if ( !resparam || resparam->param_style != ScriptParams::ScalarParam ) {
		throw Exception(ErrorScriptArgInvalid, "resource scalar param is required for %s script",
			Vm::proc_type_names[VIDEO_RESOURCE_SCRIPT]);
	}

	json_t* res_arg = get_arg_value("resource");
	json_t* in = get_arg_value("in");
	json_t* out = get_arg_value("out");

	if (!res_arg || !json_is_string(res_arg) || !strlen(json_string_value(res_arg))) {
		throw Exception(ErrorScriptArgInvalid, "resource arg is required for %s script",
			Vm::proc_type_names[VIDEO_RESOURCE_SCRIPT]);
	}

	if ( !in || !json_is_integer(in) ) {
		throw Exception(ErrorScriptArgInvalid, "in arg is required for %s script",
			Vm::proc_type_names[VIDEO_RESOURCE_SCRIPT]);
	}

	if ( !out || !json_is_integer(out) ) {
		throw Exception(ErrorScriptArgInvalid, "out arg is required for %s script",
			Vm::proc_type_names[VIDEO_RESOURCE_SCRIPT]);
	}

	string path(json_string_value(res_arg));
	mlt_producer prod = Vm::get_stream_resource(path);
	int length = mlt_producer_get_length(prod);
	int inframe = json_integer_value(in), outframe = json_integer_value(out);

	switch (inparam->pos_type) {
	case ScriptParams::FramePos:
		break;
	case ScriptParams::TimePos:
		inframe = (double)inframe / 40;
		break;
	case ScriptParams::PerctPos:
		inframe = (double)inframe / 100 * length;
		break;
	}
	if (inframe < 0) {
		inframe = length + inframe;
		if (inframe < 0 ) inframe = 0;
	}
	else {
		if (inframe >= length) {
			inframe = length - 1;
		}
	}

	switch (outparam->pos_type) {
	case ScriptParams::FramePos:
		break;
	case ScriptParams::TimePos:
		outframe = (double)outframe / 40;
		break;
	case ScriptParams::PerctPos:
		outframe = (double)outframe / 100 * length;
		break;
	}
	if (outframe < 0) {
		outframe = length + outframe;
		if (outframe < 0 ) outframe = 0;
	}
	else {
		if (outframe >= length) {
			outframe = length - 1;
		}
	}

	if ( inframe < outframe ) {
		inframe ^= outframe ^= inframe ^= outframe;
	}

	set_frame_range(inframe,outframe);

	type_spec_props->add_property("resource", res_arg);
	mlt_props->add_property("in", in);
	mlt_props->add_property("out", out);

	this->path  = path;
	//todo: check format info
	return;
}

void AudioScript::pre_judge() throw (Exception)
{
	const ScriptParam* inparam = get_param_info("in");
	const ScriptParam* outparam = get_param_info("out");
	const ScriptParam* resparam = get_param_info("resource");

	if ( !inparam || inparam->param_style != ScriptParams::PosParam ||
		!outparam || outparam->param_style != ScriptParams::PosParam) {
		throw Exception(ErrorScriptArgInvalid, "in/out position param is required for %s script",
			Vm::proc_type_names[VIDEO_RESOURCE_SCRIPT]);
	}

	if ( !resparam || resparam->param_style != ScriptParams::ScalarParam ) {
		throw Exception(ErrorScriptArgInvalid, "resource scalar param is required for %s script",
			Vm::proc_type_names[VIDEO_RESOURCE_SCRIPT]);
	}

	json_t* res_arg = get_arg_value("resource");
	json_t* in = get_arg_value("in");
	json_t* out = get_arg_value("out");

	if (!res_arg || !json_is_string(res_arg) || !strlen(json_string_value(res_arg))) {
		throw Exception(ErrorScriptArgInvalid, "resource arg is required for %s script",
			Vm::proc_type_names[VIDEO_RESOURCE_SCRIPT]);
	}

	if ( !in || !json_is_integer(in) ) {
		throw Exception(ErrorScriptArgInvalid, "in arg is required for %s script",
			Vm::proc_type_names[VIDEO_RESOURCE_SCRIPT]);
	}

	if ( !out || !json_is_integer(out) ) {
		throw Exception(ErrorScriptArgInvalid, "out arg is required for %s script",
			Vm::proc_type_names[VIDEO_RESOURCE_SCRIPT]);
	}

	string path(json_string_value(res_arg));
	mlt_producer prod = Vm::get_stream_resource(path);
	int length = mlt_producer_get_length(prod);
	int inframe = json_integer_value(in), outframe = json_integer_value(out);

	switch (inparam->pos_type) {
	case ScriptParams::FramePos:
		break;
	case ScriptParams::TimePos:
		inframe = (double)inframe / 40;
		break;
	case ScriptParams::PerctPos:
		inframe = (double)inframe / 100 * length;
		break;
	}
	if (inframe < 0) {
		inframe = length + inframe;
		if (inframe < 0 ) inframe = 0;
	}
	else {
		if (inframe >= length) {
			inframe = length - 1;
		}
	}

	switch (outparam->pos_type) {
	case ScriptParams::FramePos:
		break;
	case ScriptParams::TimePos:
		outframe = (double)outframe / 40;
		break;
	case ScriptParams::PerctPos:
		outframe = (double)outframe / 100 * length;
		break;
	}
	if (outframe < 0) {
		outframe = length + outframe;
		if (outframe < 0 ) outframe = 0;
	}
	else {
		if (outframe >= length) {
			outframe = length - 1;
		}
	}

	if ( inframe < outframe ) {
		inframe ^= outframe ^= inframe ^= outframe;
	}

	set_frame_range(inframe,outframe);

	type_spec_props->add_property("resource", res_arg);
	mlt_props->add_property("in", in);
	mlt_props->add_property("out", out);

	this->path  = path;
	//todo: check format info
	return;
}

void ImageScript::pre_judge() throw (Exception)
{
}

void GifScript::pre_judge() throw (Exception)
{
}
NMSP_END(vedit)





