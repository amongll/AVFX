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
	//mlt_producer prod = Vm::get_stream_resource(path);
	//todo: format info

	json_t* ret = json_object();
	json_object_set_new(ret, "uuid", json_string(uuid.c_str()));
	return ret;
}

AudioScript::~AudioScript()
{
}

json_t* AudioScript::compile() throw (Exception)
{
	json_t* ret = json_object();
	json_object_set_new(ret, "uuid", json_string(uuid.c_str()));
	return ret;
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

	mlt_profile profile = mlt_profile_clone(MltLoader::global_profile);
	mlt_producer prod = mlt_factory_producer(profile, "loader", path.c_str());

	if  ( prod == NULL ) {
		throw_error_v(ErrorScriptArgInvalid, "out arg is required for %s script",
			Vm::proc_type_names[VIDEO_RESOURCE_SCRIPT]);
	}

	uuid = Vm::uuid();
	MltLoader::push_mlt_registry(mlt_producer_service(prod), uuid.c_str());

	//mlt_producer prod = Vm::get_stream_resource(path);
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

	type_spec_props->add_property("uuid", JsonWrap(json_string(uuid.c_str()),1).h);
	if ( !mlt_props.get()) {
		mlt_props.reset(new ScriptProps(*this, NULL));
	}
	json_t* jv = json_integer(inframe);
	mlt_props->add_property("in", jv);
	json_decref(jv);

	jv = json_integer(outframe);
	mlt_props->add_property("out", jv);
	json_decref(jv);

	//mlt_producer_set_in_and_out(prod, inframe, outframe);
#ifdef DEBUG
	std::cout << mlt_producer_properties(prod);
#endif

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


	mlt_profile profile = mlt_profile_init(NULL);
	mlt_producer prod = mlt_factory_producer(profile, "loader", path.c_str());

	if  ( prod == NULL ) {
		throw_error_v(ErrorScriptArgInvalid, "out arg is required for %s script",
			Vm::proc_type_names[VIDEO_RESOURCE_SCRIPT]);
	}

	uuid = Vm::uuid();
	MltLoader::push_mlt_registry(mlt_producer_service(prod), uuid.c_str());

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

	if ( !type_spec_props.get() ) {
		type_spec_props.reset(new ScriptProps(*this, NULL));
	}

	type_spec_props->add_property("resource", res_arg);
	json_decref(res_arg);

	type_spec_props->add_property("uuid", JsonWrap(json_string(uuid.c_str()),1).h);

	set_frame_range(inframe,outframe);

	json_t* jv = json_integer(inframe);
	mlt_props->add_property("in", jv);
	json_decref(jv);

	jv = json_integer(outframe);
	mlt_props->add_property("out", jv);
	json_decref(jv);


	//mlt_producer_set_in_and_out(prod, inframe, outframe);
#ifdef DEBUG
	std::cout << mlt_producer_properties(prod);
#endif
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
	json_t* uuid_je = json_object_get(defines, "uuid");

	assert( je && json_is_string(je) && strlen(json_string_value(je)));
	assert( uuid_je && json_is_string(uuid_je) && strlen(json_string_value(uuid_je)));

	mlt_producer obj = MLT_PRODUCER(MltLoader::pop_mlt_registry(json_string_value(uuid_je)));
	if (obj == NULL) {
		mlt_profile profile = mlt_profile_clone(global_profile);
		obj = mlt_factory_producer(profile, "loader", json_string_value(je));
#ifdef DEBUG
		std::cout << mlt_producer_properties(obj);
#endif
	}

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

	MltSvcWrap wrap(mlt_producer_service(obj), 1);
	producer_tmp  = obj;

	parse_filters();


	wrap.obj = NULL;
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
		&SingleResourceLoader::get_video );
	MltLoader::regist_loader<SingleResourceLoader>("audio",
		&SingleResourceLoader::get_audio );
	MltLoader::regist_loader<SingleResourceLoader>("image",
		&SingleResourceLoader::get_image );
	MltLoader::regist_loader<SingleResourceLoader>( "gif",
		&SingleResourceLoader::get_gif );
	MltLoader::regist_loader<SingleResourceLoader>( "asis_producer",
		&SingleResourceLoader::get_asis_producer);
	return 4;
}

mlt_service SingleResourceLoader::get_gif(JsonWrap js)
	throw (Exception)
{
	throw_error(ErrorFeatureNotImpl);
}

mlt_service SingleResourceLoader::get_asis_producer(JsonWrap js)
	throw (Exception)
{
	json_t* defines = js.h;

	defines_tmp=js;

	json_t* je = json_object_get(defines, "resource");
	json_t* uuid_je = json_object_get(defines, "uuid");
	json_t* svc_je = json_object_get(defines, "service");

	assert( je && json_is_string(je) && strlen(json_string_value(je)));
	assert( svc_je && json_is_string(svc_je) && strlen(json_string_value(svc_je)));
	assert( uuid_je && json_is_string(uuid_je) && strlen(json_string_value(uuid_je)));

	mlt_producer obj = MLT_PRODUCER(MltLoader::pop_mlt_registry(json_string_value(uuid_je)));
	if (obj == NULL) {
		mlt_profile profile = mlt_profile_clone(global_profile);
		obj = mlt_factory_producer(profile, json_string_value(svc_je), json_string_value(je));
#ifdef DEBUG
		std::cout << mlt_producer_properties(obj);
#endif
	}

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

	MltSvcWrap wrap(mlt_producer_service(obj), 1);
	producer_tmp  = obj;

	parse_filters();


	wrap.obj = NULL;
	return mlt_producer_service(obj);
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

AsisScript::~AsisScript() {
}

json_t* AsisScript::compile() throw (Exception)
{
	return NULL;
}

void AsisScript::pre_judge() throw (Exception)
{
	ScriptProps::Property& service_prop = type_spec_props->get_property("service");
	if (!service_prop.Evaluable::finished())
		throw_error_v(ErrorScriptFmtError,"asis producer script service should be determinated");

	ScriptProps::Property& resource_prop = type_spec_props->get_property("resource");
	if (!resource_prop.Evaluable::finished())
		throw_error_v(ErrorScriptFmtError,"asis producer script resource should be determinated");

	json_t* svc_value = service_prop.compile();
	json_t* res_value = resource_prop.compile();
	JsonWrap svc_wrap(svc_value,1);
	JsonWrap res_wrap(res_value,1);

	if ( !svc_value || !json_is_string(svc_value) || !strlen(json_string_value(svc_value)))
		throw_error_v(ErrorScriptFmtError,"asis producer script service should be string value");
	if ( !res_value || !json_is_string(res_value) || !strlen(json_string_value(res_value)))
		throw_error_v(ErrorScriptFmtError,"asis producer script resource should be string value");

	mlt_profile prof = mlt_profile_clone(MltLoader::global_profile);
	mlt_producer tmp_prod = mlt_factory_producer(prof, json_string_value(svc_value), json_string_value(res_value));
	MltSvcWrap prod_wrap(mlt_producer_service(tmp_prod), 1);

	if (tmp_prod == NULL) {
		throw_error_v(ErrorRuntimeLoadFailed, "producer %s load failed", json_string_value(svc_value));
	}

	if ( mlt_props ) {
		ScriptProps::PropIter it = mlt_props->begin();
		for ( ; it!=mlt_props->end(); it++) {
			if ( it->second->Evaluable::finished() ) {
				json_t* prop_v = it->second->compile();
				JsonWrap prop_wrap(prop_v, 1);

				switch(prop_v->type) {
				case JSON_INTEGER:
					mlt_properties_set_int64(mlt_producer_properties(tmp_prod),
							it->first.c_str(), json_integer_value(prop_v));
					break;
				case JSON_REAL:
					mlt_properties_set_double(mlt_producer_properties(tmp_prod),
							it->first.c_str(), json_real_value(prop_v));
					break;
				case JSON_STRING:
					mlt_properties_set(mlt_producer_properties(tmp_prod),
							it->first.c_str(), json_string_value(prop_v));
					break;
				case JSON_TRUE:
					mlt_properties_set_int(mlt_producer_properties(tmp_prod),
							it->first.c_str(), 1);
					break;
				case JSON_FALSE:
					mlt_properties_set_int(mlt_producer_properties(tmp_prod),
							it->first.c_str(), 0);
					break;
				default:
					throw_error_v(ErrorRuntimeLoadFailed, "producer %s load failed. %s prop invalid",
							json_string_value(svc_value), it->first.c_str());
				}
			}
		}
	}

	int in = mlt_producer_get_in(tmp_prod);
	int out = mlt_producer_get_out(tmp_prod);
	set_frame_range(in, out);

	if ( !mlt_props.get()) {
		mlt_props.reset(new ScriptProps(*this, NULL));
	}
	json_t* jv = json_integer(in);
	mlt_props->add_property("in", jv);
	json_decref(jv);

	jv = json_integer(out);
	mlt_props->add_property("out", jv);
	json_decref(jv);

	string uuid = Vm::uuid();
	type_spec_props->add_property("uuid", JsonWrap(json_string(uuid.c_str()),1).h);
	prod_wrap.obj = NULL;
	MltLoader::push_mlt_registry(mlt_producer_service(tmp_prod), uuid.c_str());
 }

void AsisScript::parse_specific() throw(Exception)
{
	vector<string> specs;
	specs.push_back("resource");
	specs.push_back("service");

	parse_specific_props(specs);
}


NMSP_END(vedit)


