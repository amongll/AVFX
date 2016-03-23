/*
 * VEditMultiTrackScript.cpp
 *
 *  Created on: 2016年3月7日
 *      Author: li.lei@youku.com
 */

#include "VEditMultiTrackScript.h"
#include "VEditVM.h"

NMSP_BEGIN(vedit)

MultitrackScript::~MultitrackScript()
{
}

void MultitrackScript::pre_judge() throw (Exception)
{
	list<TrackWrap>::iterator it_track;
	for ( it_track = tracks.begin(); it_track != tracks.end(); it_track++ ) {

		it_track->track_call_results = it_track->track_calls->compile();

		list<TrackWrap::TransWrap>::iterator it_trans;
		for ( it_trans = it_track->transitions.begin();
				it_trans != it_track->transitions.end(); it_trans++) {
			it_trans->trans_call_results = it_trans->trans_calls->compile();
		}
	}

	MltSvcWrap tractor_wrp;
	mlt_tractor mlt_tracs = NULL;
	int track_idx = 0;
	for ( it_track = tracks.begin(); it_track != tracks.end(); it_track++ ) {
		mlt_service_type type = invalid_type;
		mlt_service svc_obj = NULL;

		svc_obj = MltLoader::load_mlt(it_track->track_call_results.second);
		type = mlt_service_identify(svc_obj);
		switch(type) {
		case producer_type:
		case multitrack_type:
		case tractor_type:
		case playlist_type:
			break;
		default:{
			MltSvcWrap cleanobj(svc_obj,1 );
			throw_error_v(ErrorScriptFmtError, "multitrack track callable type invalid");
		}
		}

		MltLoader::push_mlt_registry(svc_obj, it_track->track_call_results.first);

		if ( mlt_tracs == NULL ) {
			mlt_tracs = mlt_tractor_init();
			tractor_wrp.obj = mlt_tractor_service(mlt_tracs);
		}

		mlt_tractor_set_track(mlt_tracs, MLT_PRODUCER(svc_obj), track_idx++);

		list<TrackWrap::TransWrap>::iterator it_trans;
		for ( it_trans = it_track->transitions.begin();
				it_trans != it_track->transitions.end(); it_trans++) {
			svc_obj = MltLoader::load_mlt(it_trans->trans_call_results.second);
			type = mlt_service_identify(svc_obj);
			switch(type) {
			case transition_type:
				break;
			default: {
				MltSvcWrap cleanobj(svc_obj);
				throw_error_v(ErrorScriptFmtError, "multitrack track->transition callable type invalid");
			}
			}
			MltLoader::push_mlt_registry(svc_obj, it_trans->trans_call_results.first);

			mlt_field field = mlt_tractor_field(mlt_tracs);
			mlt_field_plant_transition(field, MLT_TRANSITION(svc_obj), 0, track_idx-1);
		}
	}

	if (!mlt_tracs) {
		mlt_tracs = mlt_tractor_init();
		tractor_wrp.obj = mlt_tractor_service(mlt_tracs);
	}
	mlt_tractor_refresh(mlt_tracs);

	set_frame_range(0, mlt_properties_get_position(mlt_tractor_properties(mlt_tracs),"out"));

	if ( !mlt_props.get()) {
		mlt_props.reset(new ScriptProps(*this, NULL));
	}
	json_t* jv = json_integer(frame_in);
	mlt_props->add_property("in", jv);
	json_decref(jv);

	jv = json_integer(frame_out);
	mlt_props->add_property("out", jv);
	json_decref(jv);
	return;
}

json_t* MultitrackScript::compile() throw (Exception)
{
	list<TrackWrap>::iterator it_track;
	json_t* track_jsa = NULL;
	for ( it_track = tracks.begin(); it_track != tracks.end(); it_track++ ) {
		if ( it_track->track_call_results.second.h ) {
			if (track_jsa == NULL) track_jsa = json_array();
			json_t* track_jo = it_track->track_call_results.second.h;
			json_array_append(track_jsa, track_jo);

			list<TrackWrap::TransWrap>::iterator it_trans;
			json_t* trans_jsa = NULL;
			for ( it_trans = it_track->transitions.begin() ;
				it_trans != it_track->transitions.end(); it_trans++) {
				if (trans_jsa == NULL) trans_jsa = json_array();
				json_array_append( trans_jsa, it_trans->trans_call_results.second.h);
			}

			if ( trans_jsa ) {
				json_object_set(track_jo,"transitions", trans_jsa);
			}
		}
	}

	if (track_jsa == NULL) return NULL;

	json_t* ret = json_object();
	json_object_set_new(ret, "tracks", track_jsa);
	return ret;
}

void MultitrackScript::parse_specific() throw (Exception)
{
	json_t* tracks_ja = json_object_get(defines, "tracks");
	if (!tracks_ja) return;

	if (!json_is_array(tracks_ja)) {
		throw_error_v(ErrorScriptFmtError, "tracks should be json array for multitrack script");
	}

	int asz = json_array_size(tracks_ja);
	if (asz == 0)
		return;

	for ( int i=0; i<asz; i++ ) {
		json_t* track_jso = json_array_get(tracks_ja, i);

		if ( !json_is_object(track_jso) || !json_object_size(track_jso))
			throw_error_v(ErrorScriptFmtError, "track should be object for multitrack script");

		json_t* trans_jsa = NULL;
		JsonWrap call_js_wrp;
		void *it = json_object_iter(track_jso);
		while(it) {
			const char* k = json_object_iter_key(it);
			json_t* js = json_object_iter_value(it);
			it = json_object_iter_next(track_jso, it);


			string procnm;
			if ( !strcmp(k, "transitions") ) {
				trans_jsa = js;
			}
			else if ( is_call(k, procnm) ) {
				json_t* jcp = json_object();
				json_object_set(jcp, k, js);
				call_js_wrp.h = jcp;
			}
		}

		if ( call_js_wrp.h == NULL ) {
			throw_error_v(ErrorScriptFmtError, "multitrack track callable don't exists");
		}

		ScriptCallable* call_o = new ScriptCallable(*this, call_js_wrp.h);
		tracks.push_back(TrackWrap());
		tracks.back().track_calls.reset(call_o);
		TrackWrap& track_obj = tracks.back();

		if ( !trans_jsa || i == 0 )
			continue;

		if ( !json_is_array(trans_jsa) ) {
			throw_error_v(ErrorScriptFmtError, "transition of track object should "
					"be transition callable arrays");
		}

		int asz = json_array_size(trans_jsa);
		for ( int i=0; i< asz; i++ ) {
			json_t* js = json_array_get(trans_jsa, i);

			if ( !json_is_object(js) || json_object_size(js) != 1) {
				throw_error_v(ErrorScriptFmtError, "transition of track object should "
						"be a transition callable");
			}

			void* it = json_object_iter(js);
			const char*  k = json_object_iter_key(it);
			string procnm;

			if (!is_call(k,procnm)) {
				throw_error_v(ErrorScriptFmtError, "transition of track object should "
						"be a transition callable");
			}

			ScriptCallable* call_o = new ScriptCallable(*this, js);
			track_obj.transitions.push_back(TrackWrap::TransWrap());
			TrackWrap::TransWrap& trans_obj = track_obj.transitions.back();

			trans_obj.trans_calls.reset(call_o);
		}
	}
}

void MultitrackLoader::declare()
{
	MltLoader::regist_loader<MultitrackLoader>("multitrack", &MultitrackLoader::get_tractro);
}

mlt_service MultitrackLoader::get_tractro(JsonWrap js) throw (Exception)
{
	mlt_tractor tractor_svc = NULL;
	mlt_producer track_svc = NULL;
	mlt_transition trans_svc = NULL;

	mlt_filter effect_svc = NULL;

	MltSvcWrap tractor_wrap;
	MltSvcWrap track_wrap;
	MltSvcWrap trans_wrap;

	json_t* tracks_jsa = json_object_get(js.h,"tracks");

	assert(!tracks_jsa || json_is_array(tracks_jsa));

	int tracks_count = json_array_size(tracks_jsa);
	for ( int i = 0; i < tracks_count; i++ )  {
		json_t* track_jso = json_array_get(tracks_jsa, i);
		assert(track_jso && json_is_object(track_jso) && json_object_size(track_jso));
		json_t* uuid_js = json_object_get(track_jso, "uuid");
		json_t* proctype_js = json_object_get(track_jso, "proctype");
		assert(uuid_js && json_is_string(uuid_js) && strlen(json_string_value(uuid_js)));
		assert(proctype_js && json_is_string(proctype_js) && strlen(json_string_value(proctype_js)));

		track_svc = MLT_PRODUCER(MltLoader::pop_mlt_registry(json_string_value(uuid_js)));
		if (track_svc == NULL) {
			track_svc = MLT_PRODUCER(MltLoader::load_mlt(JsonWrap(track_jso)));
		}

		if ( tractor_svc == NULL){
			tractor_svc = mlt_tractor_new();
			tractor_wrap.obj = mlt_tractor_service(tractor_svc);
		}

		mlt_tractor_set_track(tractor_svc, track_svc, i);

		json_t* trans_jsa = json_object_get(track_jso, "transitions");
		assert( !trans_jsa || json_is_array(trans_jsa) ) ;

		int trans_sz = json_array_size(trans_jsa);
		for (int ti=0; ti<trans_sz; ti++) {
			json_t* trans_jso = json_array_get(trans_jsa, ti);
			assert(trans_jso && json_is_object(trans_jso) && json_object_size(trans_jso));
			json_t* uuid_js = json_object_get(trans_jso, "uuid");
			json_t* proctype_js = json_object_get(trans_jso, "proctype");
			assert(uuid_js && json_is_string(uuid_js) && strlen(json_string_value(uuid_js)));
			assert(proctype_js && json_is_string(proctype_js) && strlen(json_string_value(proctype_js)));

			trans_svc = MLT_TRANSITION(MltLoader::pop_mlt_registry(json_string_value(uuid_js)));
			if ( trans_svc == NULL ) {
				trans_svc = MLT_TRANSITION(MltLoader::load_mlt(JsonWrap(trans_jso)));
			}

			if (trans_svc) {
				mlt_field field = mlt_tractor_field(tractor_svc);
				mlt_field_plant_transition(field, trans_svc, 0, i);
			}
		}
	}

	if ( tractor_svc == NULL ) {
		mlt_profile profile = mlt_profile_clone(MltLoader::global_profile);
		tractor_svc = mlt_tractor_new();
		tractor_wrap.obj = mlt_tractor_service(tractor_svc);
	}

	json_t* props_je = json_object_get(js.h, "props");
	mlt_properties props = mlt_tractor_properties(tractor_svc);

	json_t* je = json_object_get(js.h, "props");
	if (je && json_is_object(je) && json_object_size(je)) {

		void* it = json_object_iter(je);
		while(it) {
			const char* k = json_object_iter_key(it);
			json_t* prop_je = json_object_iter_value(it);
			it = json_object_iter_next(je, it);

			if ( json_is_object(prop_je) || json_is_array(prop_je))
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

	json_t* filters_ja = json_object_get(js.h,"effects");
	if (!filters_ja || !json_is_array(filters_ja) || !json_array_size(filters_ja)  ) {
		tractor_wrap.obj = NULL;
		return mlt_tractor_service(tractor_svc);
	}

	int aze = json_array_size(filters_ja);
	for ( int i=0; i<aze; i++ ) {
		json_t* jae = json_array_get(filters_ja, i);
		mlt_service mlt_obj = MltLoader::load_mlt(JsonWrap(jae));
		assert( filter_type == mlt_service_identify(mlt_obj));
		mlt_filter filter = MLT_FILTER(mlt_obj);
		mlt_producer_attach(mlt_tractor_producer(tractor_svc), filter);
	}

#ifdef DEBUG

	std::cout << "======filter========" << json_string_value(json_object_get(js.h,"uuid"))<< "===filter====" <<endl;
	std::cout << mlt_tractor_properties(tractor_svc);
	std::cout << "##############################################################" << endl;
#endif

	tractor_wrap.obj = NULL;
	return mlt_tractor_service(tractor_svc);
}


NMSP_END(vedit)
