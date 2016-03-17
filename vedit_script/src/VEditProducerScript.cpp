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
		throw_error_v(ErrorScriptArgInvalid, "in/out position param is required for %s script",
			Vm::proc_type_names[VIDEO_RESOURCE_SCRIPT]);
	}

	if ( !resparam || resparam->param_style != ScriptParams::ScalarParam ) {
		throw_error_v(ErrorScriptArgInvalid, "resource scalar param is required for %s script",
			Vm::proc_type_names[VIDEO_RESOURCE_SCRIPT]);
	}

	json_t* res_arg = get_arg_value("resource");
	json_t* in = get_arg_value("in");
	json_t* out = get_arg_value("out");

	if (!res_arg || !json_is_string(res_arg) || !strlen(json_string_value(res_arg))) {
		throw_error_v(ErrorScriptArgInvalid, "resource arg is required for %s script",
			Vm::proc_type_names[VIDEO_RESOURCE_SCRIPT]);
	}

	if ( !in || !json_is_integer(in) ) {
		throw_error_v(ErrorScriptArgInvalid, "in arg is required for %s script",
			Vm::proc_type_names[VIDEO_RESOURCE_SCRIPT]);
	}

	if ( !out || !json_is_integer(out) ) {
		throw_error_v(ErrorScriptArgInvalid, "out arg is required for %s script",
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
		throw_error_v(ErrorScriptArgInvalid, "in/out position param is required for %s script",
			Vm::proc_type_names[VIDEO_RESOURCE_SCRIPT]);
	}

	if ( !resparam || resparam->param_style != ScriptParams::ScalarParam ) {
		throw_error_v(ErrorScriptArgInvalid, "resource scalar param is required for %s script",
			Vm::proc_type_names[VIDEO_RESOURCE_SCRIPT]);
	}

	json_t* res_arg = get_arg_value("resource");
	json_t* in = get_arg_value("in");
	json_t* out = get_arg_value("out");

	if (!res_arg || !json_is_string(res_arg) || !strlen(json_string_value(res_arg))) {
		throw_error_v(ErrorScriptArgInvalid, "resource arg is required for %s script",
			Vm::proc_type_names[VIDEO_RESOURCE_SCRIPT]);
	}

	if ( !in || !json_is_integer(in) ) {
		throw_error_v(ErrorScriptArgInvalid, "in arg is required for %s script",
			Vm::proc_type_names[VIDEO_RESOURCE_SCRIPT]);
	}

	if ( !out || !json_is_integer(out) ) {
		throw_error_v(ErrorScriptArgInvalid, "out arg is required for %s script",
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

mlt_service SingleResourceLoader::get_video(JsonWrap js)
	throw (Exception)
{
	json_t* defines = js.h;

	defines_tmp=js;

	json_t* je = json_object_get(defines, "resource");

	assert( je && json_is_string(je) && strlen(json_string_value(je)));

	mlt_profile profile = mlt_profile_init(NULL);
	mlt_producer obj = mlt_factory_producer(profile, "loader", json_string_value(je));

	if ( !obj ) {
		throw_error_v(ErrorRuntimeLoadFailed, "video producer load failed:%s", json_string_value(je));
	}

	mlt_properties props = mlt_producer_properties(obj);

	je = json_object_get(defines, "props");
	if (je && json_is_object(je) && json_object_size(je)) {

		json_t* inj = json_object_get(je,"in"), *outj = json_object_get(je,"out");
		assert(inj && outj && json_is_integer(inj) && json_is_integer(outj));

		mlt_producer_set_in_and_out(obj,json_integer_value(inj), json_integer_value(outj));

		void* it = json_object_iter(je);
		while(it) {
			const char* k = json_object_iter_key(it);
			json_t* prop_je = json_object_iter_value(it);
			it = json_object_iter_next(je, it);

			if ( json_is_object(prop_je) || json_is_array(prop_je))
				continue;

			if ( !strcmp(k,"in") || !strcmp(k,"out"))
				continue;

			switch(prop_je->type) {
			case JSON_INTEGER:
				mlt_properties_set_int64(props, k, json_integer_value(prop_je));
				break;
			case JSON_REAL:
				mlt_properties_set_double(props, k, json_real_value(prop_je));
				break;
			case JSON_STRING:
				mlt_properties_set(props, k, json_string_value(prop_je));
				break;
			case JSON_TRUE:
				mlt_properties_set_int(props,k, 1);
				break;
			case JSON_FALSE:
				mlt_properties_set_int(props,k, 0);
				break;
			default:
				break;
			}
		}
	}

	producer_tmp  = obj;

	parse_filters();

	return mlt_producer_service(obj);
}

mlt_service SingleResourceLoader::get_audio(JsonWrap js)
	throw (Exception)
{
	throw_error(ErrorFeatureNotImpl);
}

mlt_service SingleResourceLoader::get_image(JsonWrap js)
	throw (Exception)
{
	throw_error(ErrorFeatureNotImpl);
}

int SingleResourceLoader::declare() {
	MltLoader::regist_loader<SingleResourceLoader>("video",
		static_cast<MltLoader::LoadMltMemFp>(&SingleResourceLoader::get_video) );
	MltLoader::regist_loader<SingleResourceLoader>("audio",
		static_cast<MltLoader::LoadMltMemFp>(&SingleResourceLoader::get_audio) );
	MltLoader::regist_loader<SingleResourceLoader>("image",
		static_cast<MltLoader::LoadMltMemFp>(&SingleResourceLoader::get_image) );
	MltLoader::regist_loader<SingleResourceLoader>( "gif",
		static_cast<MltLoader::LoadMltMemFp>(&SingleResourceLoader::get_gif) );
	return 4;
}

mlt_service SingleResourceLoader::get_gif(JsonWrap js)
	throw (Exception)
{
	throw_error(ErrorFeatureNotImpl);
}

void SingleResourceLoader::parse_filters() throw (Exception)
{
	json_t* ja = json_object_get(defines_tmp.h, "effects");
	if (!ja || !json_is_array(ja) || !json_array_size(ja)  ) return;

	int aze = json_array_size(ja);
	for ( int i=0; i<aze; i++ ) {
		json_t* jae = json_array_get(ja, i);
		mlt_filter filter = get_filter(jae);

		mlt_producer_attach(producer_tmp, filter);
	}
}

SingleResourceLoader::~SingleResourceLoader()
{
	if ( producer_tmp ) mlt_producer_close(producer_tmp);
}

mlt_filter SingleResourceLoader::get_filter(json_t* defines) throw (Exception)
{
	if ( !json_is_object(defines) || !json_object_size(defines) ) {
		throw_error_v(ErrorRuntimeLoadFailed, "effect json serialization format error");
	}

	mlt_filter ret = (mlt_filter)MltLoader::load_mlt(JsonWrap(defines));
	return ret;
}


NMSP_END(vedit)



