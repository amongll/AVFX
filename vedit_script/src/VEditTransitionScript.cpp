/*
 * VEditTransitionScript.cpp
 *
 *  Created on: 2016年3月7日
 *      Author: li.lei@youku.com
 */

#include "VEditTransitionScript.h"
#include "VEditVM.h"


NMSP_BEGIN(vedit)

TransitionScript::~TransitionScript()
{
}

void TransitionScript::pre_judge() throw (Exception)
{
	assert(type_spec_props);

	ScriptProps::Property& filter_prop = type_spec_props->get_property("transition");
	if (!filter_prop.Evaluable::finished()) {
		throw_error_v(ErrorScriptFmtError, "transition name do not support macro/param expandable");
	}

	const ScriptParam* inparam = get_param_info("in");
	const ScriptParam* outparam = get_param_info("out");

	if ( !inparam || inparam->param_style != ScriptParams::PosParam ||
			!outparam || outparam->param_style != ScriptParams::PosParam) {
		throw_error_v(ErrorScriptArgInvalid, "in/out position param is required for %s script",
				Vm::proc_type_names[TRANSITION_SCRIPT]);
	}

	json_t* in = get_arg_value("in");
	json_t* out = get_arg_value("out");

	if ( !in || !json_is_integer(in) ) {
		throw_error_v(ErrorScriptArgInvalid, "in arg is required for %s script",
				Vm::proc_type_names[TRANSITION_SCRIPT]);
	}

	if ( !out || !json_is_integer(out) ) {
		throw_error_v(ErrorScriptArgInvalid, "out arg is required for %s script",
				Vm::proc_type_names[TRANSITION_SCRIPT]);
	}

	int inframe = json_integer_value(in), outframe = json_integer_value(out);
	json_decref(in);
	json_decref(out);

	if ( inframe < 0 || outframe < 0) {
		throw_error_v(ErrorScriptArgInvalid, "in/out arg should be positive position for %s script",
				Vm::proc_type_names[TRANSITION_SCRIPT]);
	}

	switch (inparam->pos_type) {
	case ScriptParams::TimePos: {
		if ( inframe % 40 == 0 && inframe > 0 )
			inframe = inframe/40 - 1;
		else
			inframe = inframe / 40 ;
		break;
	}
	}

	switch (outparam->pos_type) {
	case ScriptParams::TimePos: {
		if ( outframe % 40 == 0 && outframe > 0) {
			outframe = outframe/40 - 1;
		}
		else {
			outframe = outframe/40;
		}
		break;
	}
	}

	if ( inframe > outframe ) {
		inframe ^= outframe ^= inframe ^= outframe;
	}

	set_frame_range(inframe,outframe);

	const ScriptParam* resparam = get_param_info("argument");

	if ( resparam && resparam->param_style != ScriptParams::ScalarParam ) {
		throw_error_v(ErrorScriptArgInvalid, "argument param should be scalre if exists for %s script",
				Vm::proc_type_names[TRANSITION_SCRIPT]);
	}

	json_t* res_arg = get_arg_value("argument");

	if (res_arg) {
		if ( !type_spec_props.get() ) {
			type_spec_props.reset(new ScriptProps(*this, NULL));
		}

		type_spec_props->add_property("argument", res_arg);
		json_decref(res_arg);
	}
	if ( !mlt_props.get()) {
		mlt_props.reset(new ScriptProps(*this, NULL));
	}
	json_t* jv = json_integer(inframe);
	mlt_props->add_property("in", jv);
	json_decref(jv);

	jv = json_integer(outframe);
	mlt_props->add_property("out", jv);
	json_decref(jv);

	return;
}

json_t* TransitionScript::compile() throw (Exception)
{
	return NULL;
}

void TransitionScript::parse_specific() throw (Exception)
{
	vector<string> specs;
	specs.push_back("transition");
	specs.push_back("argument");
	parse_specific_props(specs);
}


void TransitionLoader::declare()
{
	MltLoader::regist_loader<TransitionLoader>("transition", &TransitionLoader::get_transition);
}

mlt_service TransitionLoader::get_transition(JsonWrap js) throw (Exception)
{
	json_t* defines = js.h;
	json_t* je = json_object_get(defines, "argument");

	assert( !je || json_is_string(je) );

	const char* arg_str = je&&strlen(json_string_value(je))?json_string_value(je):NULL;

	je = json_object_get(defines, "transition");

	assert(je && json_is_string(je) && strlen(json_string_value(je)));

	const char* filter_id = json_string_value(je);

	mlt_profile profile = mlt_profile_init(NULL);
	mlt_transition obj = mlt_factory_transition(profile, filter_id, arg_str);

	if ( !obj ) {
		throw_error_v(ErrorRuntimeLoadFailed, "transition load failed:%s", filter_id);
	}

	mlt_properties props = mlt_transition_properties(obj);

	je = json_object_get(defines, "props");
	if (je && json_is_object(je) && json_object_size(je)) {

		json_t* inj = json_object_get(je,"in"), *outj = json_object_get(je,"out");
		assert(inj && outj && json_is_integer(inj) && json_is_integer(outj));

		mlt_transition_set_in_and_out(obj,json_integer_value(inj), json_integer_value(outj));

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

#ifdef DEBUG
	std::cout << "==============" << json_string_value(json_object_get(js.h,"uuid"))<< "=======" <<endl;
	std::cout << mlt_transition_properties(obj);
#endif

	return mlt_transition_service(obj);
}


NMSP_END(vedit)


