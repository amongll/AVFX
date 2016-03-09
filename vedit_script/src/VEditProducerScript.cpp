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

	json_decref(in);
	json_decref(out);

	switch (inparam->pos_type) {
	case ScriptParams::FramePos:
		if ( inframe == -1  || inframe >= length) {
			inframe = length - 1;
		}
		else if ( inframe < 0 ) {
			inframe = length + inframe;
			if (inframe < 0)inframe = 0;
		}
		break;
	case ScriptParams::TimePos: {
		int total_time = length * 40;
		if ( inframe == -1 ) {
			inframe = length - 1;
		}
		else if ( inframe >= 0  ){
			if ( inframe > total_time) {
				inframe = length -1;
			}
			else {
				inframe = (double)inframe / 40 ;
			}
		}
		else {
			inframe = total_time + inframe;
			if ( inframe < 0 ) {
				inframe = length - 1;
			}
			else {
				inframe = (double)inframe/40;
			}
		}
	}
		break;
	}


	switch (outparam->pos_type) {
	case ScriptParams::FramePos:
		if ( outframe == -1  || outframe >= length) {
			outframe = length - 1;
		}
		else if ( outframe < 0 ) {
			outframe = length + outframe;
			if (outframe < 0)outframe = 0;
		}
		break;
	case ScriptParams::TimePos: {
		int total_time = length * 40;
		if ( outframe == -1 ) {
			outframe = length - 1;
		}
		else if ( outframe >= 0  ){
			if ( outframe > total_time) {
				outframe = length -1;
			}
			else {
				outframe = (double)outframe / 40 ;
			}
		}
		else {
			outframe = total_time + outframe;
			if ( outframe < 0 ) {
				outframe = length - 1;
			}
			else {
				outframe = (double)outframe/40;
			}
		}
	}
		break;
	}

	if ( inframe > outframe ) {
		inframe ^= outframe ^= inframe ^= outframe;
	}

	set_frame_range(inframe,outframe);

	if ( !type_spec_props.get() ) {
		type_spec_props.reset(new ScriptProps(*this, NULL));
	}

	type_spec_props->add_property("resource", res_arg);
	json_decref(res_arg);
	if ( !mlt_props.get()) {
		mlt_props.reset(new ScriptProps(*this, NULL));
	}
	json_t* jv = json_integer(inframe);
	mlt_props->add_property("in", jv);
	json_decref(jv);

	jv = json_integer(outframe);
	mlt_props->add_property("out", jv);
	json_decref(jv);

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

	json_decref(in);
	json_decref(out);

	switch (inparam->pos_type) {
	case ScriptParams::FramePos:
		if ( inframe == -1  || inframe >= length) {
			inframe = length - 1;
		}
		else if ( inframe < 0 ) {
			inframe = length + inframe;
			if (inframe < 0)inframe = 0;
		}
		break;
	case ScriptParams::TimePos: {
		int total_time = length * 40;
		if ( inframe == -1 ) {
			inframe = length - 1;
		}
		else if ( inframe >= 0  ){
			if ( inframe > total_time) {
				inframe = length -1;
			}
			else {
				inframe = (double)inframe / 40 ;
			}
		}
		else {
			inframe = total_time + inframe;
			if ( inframe < 0 ) {
				inframe = length - 1;
			}
			else {
				inframe = (double)inframe/40;
			}
		}
	}
		break;
	}


	switch (outparam->pos_type) {
	case ScriptParams::FramePos:
		if ( outframe == -1  || outframe >= length) {
			outframe = length - 1;
		}
		else if ( outframe < 0 ) {
			outframe = length + outframe;
			if (outframe < 0)outframe = 0;
		}
		break;
	case ScriptParams::TimePos: {
		int total_time = length * 40;
		if ( outframe == -1 ) {
			outframe = length - 1;
		}
		else if ( outframe >= 0  ){
			if ( outframe > total_time) {
				outframe = length -1;
			}
			else {
				outframe = (double)outframe / 40 ;
			}
		}
		else {
			outframe = total_time + outframe;
			if ( outframe < 0 ) {
				outframe = length - 1;
			}
			else {
				outframe = (double)outframe/40;
			}
		}
	}
		break;
	}

	if ( inframe > outframe ) {
		inframe ^= outframe ^= inframe ^= outframe;
	}

	set_frame_range(inframe,outframe);

	type_spec_props->add_property("resource", res_arg);
	json_decref(res_arg);
	json_t* jv = json_integer(inframe);
	mlt_props->add_property("in", jv);
	json_decref(jv);

	jv = json_integer(outframe);
	mlt_props->add_property("out", jv);
	json_decref(jv);

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





