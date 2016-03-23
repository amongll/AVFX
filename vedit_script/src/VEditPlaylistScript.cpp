/*
 * VEditPlaylistScript.cpp
 *
 *  Created on: 2016年3月7日
 *      Author: li.lei@youku.com
 */

#include "VEditPlaylistScript.h"
#include "VEditVM.h"

NMSP_BEGIN(vedit)

PlaylistScript::~PlaylistScript()
{
}

json_t* PlaylistScript::compile() throw (Exception)
{
	if (slices.size() == 0) return NULL;
	SliceIter it;
	json_t* slices_ja  = NULL;
	JsonWrap slices_jawrap;
	for ( it = slices.begin(); it != slices.end(); it++ ) {
		json_t* slice_jso = NULL;
		JsonWrap slice_jswrap;
		if ( it->call && it->call_result.second.h ) {
			if (!slice_jso) {
				slice_jso = json_object();
				slice_jswrap.h = slice_jso;
			}

			const char* k = NULL;
			json_t* je = NULL;
			json_object_foreach(it->call_result.second.h, k, je)
			{
				json_object_set(slice_jso, k, je);
			}
		}

		if ( it->ctrl_result.h ) {
			if (!slice_jso) {
				slice_jso = json_object();
				slice_jswrap.h = slice_jso;
			}
			json_object_set(slice_jso, "ctrl", it->ctrl_result.h);

			if ( it->call_result.second.h == NULL ) {

				json_t* blank_je = json_object_get(it->ctrl_result.h, "blank_time");
				if (blank_je == NULL) blank_je = json_object_get(it->ctrl_result.h, "blank_frames");

				if (blank_je == NULL) {
					throw_error_v(ErrorScriptFmtError, "playlist blank slices requires blank_time/blank_frames");
				}

				json_t* uuid_je = json_object_get(it->ctrl_result.h, "uuid");
				if ( uuid_je == NULL ) {
					json_object_set_new(slice_jso, "uuid", json_string(Vm::uuid().c_str()));
				}
				else {
					json_incref(uuid_je);
					json_object_del(it->ctrl_result.h, "uuid");
					json_object_set_new(slice_jso, "uuid", uuid_je);
				}
			}
		}

		if (slice_jso) {
			if (!slices_ja) {
				slices_ja = json_array();
				slices_jawrap.h = slices_ja;
			}
			json_array_append(slices_ja, slice_jso);
		}

		MixIter mit;
		json_t* mixes_ja = NULL;
		for ( mit = it->mixes.begin(); mit != it->mixes.end(); mit++) {
			if (mixes_ja==NULL) mixes_ja = json_array();
			json_array_append(mixes_ja, mit->call_result.second.h);
		}

		if (mixes_ja) {
			json_object_set_new(slice_jso, "mixes", mixes_ja);

			if ( it->ctrl_result.h == NULL) {
				throw_error_v(ErrorScriptFmtError, "playlist slice mix require ctrl->mix_frames/mix_time");
			}

			json_t* mixframes_je = json_object_get(it->ctrl_result.h, "mix_frames");
			if (mixframes_je == NULL) {
				mixframes_je = json_object_get(it->ctrl_result.h, "mix_time");
			}
			if (!mixframes_je) {
				throw_error_v(ErrorScriptFmtError, "playlist slice mix require ctrl->mix_frames/mix_time");
			}
		}
	}

	if ( slices_ja ) {
		json_t* ret = json_object();
		json_object_set(ret, "slices", slices_ja);
		return ret;
	}
	return NULL;
}

void PlaylistScript::parse_specific() throw (Exception)
{
	json_t* slices_ja = json_object_get(defines, "slices");
	if (!slices_ja) return;
	if ( !json_is_array(slices_ja) ) {
		throw_error_v(ErrorScriptFmtError,"playlist script slices format error");
	}
	int asz = json_array_size(slices_ja);
	if ( !asz ) return;

	for ( int i=0; i< asz; i++ ) {
		json_t* ae = json_array_get(slices_ja, i);
		json_t* call_je = NULL, *ctrl_je = NULL, *mixes_je = NULL;
		void* it = json_object_iter(ae);
		while(it) {
			const char* k = json_object_iter_key(it);
			json_t* aee = json_object_iter_value(it);
			it = json_object_iter_next(ae, it);
			string procnm;
			if ( is_call(k, procnm) ) {
				call_je = json_object();
				json_object_set(call_je, k, aee);
			}
			else if ( !strcasecmp( k, "ctrl") ) {
				ctrl_je = aee;
			}
			else if ( !strcmp( k, "mixes" ) ) {
				mixes_je = aee;
			}
		}

		JsonWrap call_je_wrap(call_je);
		ScriptCallable* call_obj = NULL;
		ScriptProps* ctrl_props = NULL;

		if (ctrl_je == NULL && call_je == NULL) {
			throw_error_v(ErrorScriptFmtError,"playlist script slice should be callable");
		}

		slices.push_back(SliceWrap());
		SliceWrap& slice_wrap = slices.back();

		if ( call_je ) {
			call_obj = new ScriptCallable(*this, call_je);
			slice_wrap.call.reset(call_obj);
		}

		if (ctrl_je) {
			vector<string> specs ;
			specs.push_back("blank_frames");
			specs.push_back("blank_time");
			specs.push_back("repeat");
			specs.push_back("mix_frames");
			specs.push_back("mix_time");
			specs.push_back("uuid");
			ctrl_props = new ScriptProps(*this, ctrl_je, specs);
			slice_wrap.ctrls.reset(ctrl_props);
		}

		if (!mixes_je || i == 0) continue;

		if ( !json_is_array(mixes_je) ) {
			throw_error_v(ErrorScriptFmtError,"playlist script slices mixes"
				" should be callable array");
		}

		int mix_asz = json_array_size(mixes_je);
		for ( int mi = 0; mi < mix_asz; mi++ ) {
			json_t* mix_je = json_array_get(mixes_je, mi);
			if ( !json_is_object(mix_je) && 1 != json_object_size(mix_je)) {
				throw_error_v(ErrorScriptFmtError, "playlist script slice mixe"
					" should be callable");
			}

			void* it = json_object_iter(mix_je);
			const char* k = json_object_iter_key(it);
			string procnm2;
			if (!is_call(k, procnm2)) {
				throw_error_v(ErrorScriptFmtError, "playlist script slice mixe"
					" should be callable");
			}

			slice_wrap.mixes.push_back(SliceWrap::MixWrap());
			SliceWrap::MixWrap& mix_wrap = slice_wrap.mixes.back();

			call_obj = new ScriptCallable(*this, mix_je);
			mix_wrap.call.reset(call_obj);
		}
	}
}

void PlaylistScript::pre_judge() throw (Exception)
{
	SliceIter it;
	for ( it = slices.begin(); it != slices.end(); it++ ) {
		if ( it->call ) {
			it->call_result = it->call->compile();
		}
		if ( it->ctrls ) {
			json_t* ctl_js = it->ctrls->compile();
			it->ctrl_result.h = ctl_js;
		}

		MixIter mit;
		for ( mit = it->mixes.begin(); mit != it->mixes.end(); mit++) {
			mit->call_result = mit->call->compile();
		}
	}

	/**
	 * 临时组件一个playlist对象，来决定playlist的持续时长。
	 * 组成playlist的slice和transtion对象会被临时注册。
	 */
	mlt_playlist playlist = NULL;
	MltSvcWrap playlist_wrap;
	mlt_transition pending_transition = NULL;
	int pending_mix_len = 0;
	int slice_idx = 0;
	for ( it = slices.begin(); it != slices.end(); it++, slice_idx++ ) {
		mlt_service svc_obj = NULL;
		mlt_service_type type = invalid_type;
		if ( it->call_result.second.h ) {
			svc_obj = MltLoader::load_mlt(it->call_result.second);
			type = mlt_service_identify(svc_obj);
			switch(type) {
			case producer_type:
			case playlist_type:
			case tractor_type:
			case multitrack_type:
				break;
			default: {
				MltSvcWrap cleanobj(svc_obj, 1);
				throw_error_v(ErrorScriptFmtError, "Playlist script slice call type error");
			}
			}
		}

		if ( svc_obj == NULL ) {
			if ( it->ctrl_result.h == NULL) {
				throw_error_v(ErrorScriptFmtError, "playlist script slice should callable or blank");
			}

			bool timeflag = false;
			json_t* blk_je = json_object_get(it->ctrl_result.h, "blank_frames");
			if ( blk_je == NULL || !json_is_integer(blk_je) ) {
				blk_je = json_object_get(it->ctrl_result.h, "blank_time");
				if ( blk_je && !json_is_integer(blk_je))
					blk_je = NULL;
				if (blk_je) timeflag = true;
			}
			if (blk_je == NULL) {
				throw_error_v(ErrorScriptFmtError, "playlist script slice should callable or blank");
			}

			int blkv = json_integer_value(blk_je);
			if (blkv <= 0) {
				throw_error_v(ErrorScriptFmtError, "playlist blank slice should be positive duration");
			}
			if (timeflag){
				blkv /= 40;
			}

			if (playlist == NULL) {
				mlt_profile profile = mlt_profile_clone(MltLoader::global_profile);
				playlist = mlt_playlist_new(profile);
				playlist_wrap.obj = mlt_playlist_service(playlist);
			}

			mlt_playlist_blank(playlist, blkv);
		}
		else {

			if (playlist == NULL) {
				mlt_profile profile = mlt_profile_clone(MltLoader::global_profile);
				playlist = mlt_playlist_new(profile);
				playlist_wrap.obj = mlt_playlist_service(playlist);
			}

			mlt_playlist_append(playlist, MLT_PRODUCER(svc_obj));
			MltLoader::push_mlt_registry(svc_obj, it->call_result.first);
		}

		MixIter mit;
		if ( it->mixes.size() == 0 )
			continue;

		if ( it->ctrl_result.h == NULL  ) {
			throw_error_v(ErrorScriptFmtError, "playlist slice mix require mix_frames/mix_time ctrl");
		}

		bool mltime = false;
		json_t* mlje = json_object_get(it->ctrl_result.h, "mix_frames");
		if (mlje == NULL) {
			mlje = json_object_get(it->ctrl_result.h, "mix_time");
			mltime = true;
		}
		if ( !mlje || !json_is_integer(mlje) ) {
			throw_error_v(ErrorScriptFmtError, "playlist slice mix require mix_frames/mix_time ctrl");
		}

		it->mix_len = json_integer_value(mlje);
		if (it->mix_len <= 0 ) {
			throw_error_v(ErrorScriptFmtError, "playlist slice mix require mix_frames/mix_time ctrl");
		}

		if ( mltime) {
			it->mix_len /= 40;
		}

		int j=0;

		mlt_tractor mix_tractor = NULL;
		mlt_field field = NULL;
		if (mlt_playlist_mix(playlist, mlt_playlist_count(playlist) - 2, it->mix_len, NULL) == 0) {
			mlt_playlist_clip_info info;
			mlt_playlist_get_clip_info( playlist, &info, mlt_playlist_count( playlist ) - 1 );
			if ( mlt_properties_get_data( ( mlt_properties )info.producer, "mlt_mix", NULL ) == NULL )
				mlt_playlist_get_clip_info( playlist, &info, mlt_playlist_count( playlist ) - 2 );
			mix_tractor = ( mlt_tractor )mlt_properties_get_data( ( mlt_properties )info.producer, "mlt_mix", NULL );
			field = mlt_tractor_field(mix_tractor);
		}

		if (field == NULL)
			continue;

		for ( mit = it->mixes.begin(); mit != it->mixes.end(); mit++,j++ ) {
			mlt_service mix_obj = MltLoader::load_mlt(mit->call_result.second);

			if ( transition_type != mlt_service_identify(mix_obj)) {
				MltSvcWrap cleanobj(mix_obj, 1);
				throw_error_v(ErrorScriptFmtError, "playlist slice mix callable type error");
			}
			MltLoader::push_mlt_registry(mix_obj, mit->call_result.first);
			//mlt_field_plant_transition(field, MLT_TRANSITION(mix_obj), 0 , 1);
			//mlt_transition_set_in_and_out(MLT_TRANSITION(mix_obj), 0 , it->mix_len - 1);
		}
	}

	if (playlist == NULL) {
		set_frame_range(0,0);
	}
	else
		set_frame_range(0, mlt_producer_get_out(mlt_playlist_producer(playlist)));

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

void PlaylistLoader::declare()
{
	MltLoader::regist_loader<PlaylistLoader>("playlist", &PlaylistLoader::load_playlist);
}

mlt_service PlaylistLoader::load_playlist(JsonWrap js) throw (Exception)
{
	json_t* slices_ja = json_object_get(js.h, "slices");
	mlt_playlist playlist = NULL;
	json_t* ctrl_je = NULL;
	mlt_producer slice_mlt = NULL;
	json_t* mixes_ja = NULL;
	mlt_service mlt_obj = NULL;
	mlt_transition mix = NULL;
	MltSvcWrap playlist_wrap;
	MltSvcWrap mix_wrap;

	int slices_sz = json_array_size(slices_ja);
	for ( int i=0; i<slices_sz; i++) {
		json_t* mlt_e = json_array_get(slices_ja, i);
		assert ( json_is_object(mlt_e) && json_object_size(mlt_e) );

		JsonWrap jswrap(json_copy(mlt_e), 1);
		mlt_e = jswrap.h;

		ctrl_je = json_object_get(mlt_e, "ctrl");
		if (ctrl_je) {
			json_incref(ctrl_je);
			json_object_del(mlt_e, "ctrl");
		}
		JsonWrap ctrlwrap(ctrl_je, 1);

		mixes_ja = json_object_get(mlt_e, "mixes");
		if ( mixes_ja) {
			json_incref(mixes_ja);
			json_object_del(mlt_e, "mixes");
		}
		JsonWrap mixeswrap(mixes_ja, 1);

		mlt_obj = NULL;
		slice_mlt = NULL;
		mlt_service_type type = invalid_type;

		json_t* uuid_je = json_object_get(mlt_e, "uuid");
		json_t* proctype_je = json_object_get(mlt_e, "proctype");

		if ( uuid_je && proctype_je ) {
			assert(json_is_string(uuid_je)&& strlen(json_string_value(uuid_je)));
			assert(json_is_string(proctype_je)&& strlen(json_string_value(proctype_je)));

			mlt_obj = MltLoader::pop_mlt_registry(json_string_value(uuid_je));
			if (mlt_obj == NULL) {
				mlt_obj = MltLoader::load_mlt(jswrap);
			}

			if ( mlt_obj == NULL) {
				throw_error_v(ErrorRuntimeLoadFailed, "playlist slice entry load failed");
			}

			type = mlt_service_identify(mlt_obj);
			switch(type) {
			case producer_type:
			case playlist_type:
			case tractor_type:
			case multitrack_type:
				slice_mlt = MLT_PRODUCER(mlt_obj);
				break;
			default:
				throw_error_v(ErrorRuntimeLoadFailed, "playlist slice type error");
				break;
			}
		}

		if (slice_mlt) {
			if (playlist==NULL) {
				mlt_profile profile = mlt_profile_clone(global_profile);
				playlist = mlt_playlist_new(profile);
				playlist_wrap.obj = mlt_playlist_service(playlist);
			}
			mlt_playlist_append(playlist,slice_mlt);

			if ( ctrl_je ) {
				json_t* repeat_je = json_object_get(ctrl_je, "repeat");
				if ( repeat_je && json_is_integer(repeat_je) && json_integer_value(repeat_je) > 0) {
					int idx = mlt_playlist_count(playlist) - 1;
					mlt_playlist_repeat_clip(playlist, idx , json_integer_value(repeat_je));
				}
			}
		}
		else if (ctrl_je) {
			if (playlist==NULL) {
				mlt_profile profile = mlt_profile_init(NULL);
				playlist = mlt_playlist_new(profile);
				playlist_wrap.obj = mlt_playlist_service(playlist);
			}

			bool timeflag = false;
			json_t* blank_je = json_object_get(ctrl_je, "blank_frames");
			if ( !blank_je || !json_is_integer(blank_je)) {
				blank_je = json_object_get(ctrl_je, "blank_time");
				timeflag = true;
			}

			assert( blank_je && json_is_integer(blank_je) && json_integer_value(blank_je) > 0);

			int blk = json_integer_value(blank_je);

			if (timeflag) {
				blk = blk/40;
			}

			mlt_playlist_blank(playlist, blk);
		}

		if (mixes_ja) {
			assert(ctrl_je);
			bool mix_len_time=false;
			json_t* mix_len_je = json_object_get(ctrl_je, "mix_frames");
			if (mix_len_je==NULL) {
				mix_len_je = json_object_get(ctrl_je, "mix_time");
				mix_len_time = true;
			}
			assert(mix_len_je && json_is_integer(mix_len_je) && json_integer_value(mix_len_je)>0);
			int mix_len = json_integer_value(mix_len_je);
			if(mix_len_time) {
				mix_len = mix_len/40;
			}

			mlt_tractor mix_tractor = NULL;
			mlt_field field = NULL;
			if (mlt_playlist_mix(playlist, mlt_playlist_count(playlist) - 2, mix_len, NULL) == 0) {
				mlt_playlist_clip_info info;
				mlt_playlist_get_clip_info( playlist, &info, mlt_playlist_count( playlist ) - 1 );
				if ( mlt_properties_get_data( ( mlt_properties )info.producer, "mlt_mix", NULL ) == NULL )
					mlt_playlist_get_clip_info( playlist, &info, mlt_playlist_count( playlist ) - 2 );
				mix_tractor = ( mlt_tractor )mlt_properties_get_data( ( mlt_properties )info.producer, "mlt_mix", NULL );
				field = mlt_tractor_field(mix_tractor);
			}


			int asz = json_array_size(mixes_ja);
			for ( int i=0; i<asz; i++) {
				json_t* mix_jso = json_array_get(mixes_ja, i);
				json_t* proctype_je = json_object_get(mix_jso, "proctype");
				json_t* uuid_je = json_object_get(mix_jso, "uuid");

				assert(proctype_je && json_is_string(proctype_je) && strlen(json_string_value(proctype_je)));
				assert(uuid_je && json_is_string(uuid_je) && strlen(json_string_value(uuid_je)));

				mlt_obj = MltLoader::pop_mlt_registry(json_string_value(uuid_je));
				if (mlt_obj == NULL) {
					mlt_obj = MltLoader::load_mlt(JsonWrap(mix_jso));
				}

				type = mlt_service_identify(mlt_obj);
				if (type != transition_type) {
					throw_error_v(ErrorRuntimeLoadFailed, "playlist mixes type error");
				}

				mlt_transition trans_mlt = MLT_TRANSITION(mlt_obj);
				if (field) {
					mlt_field_plant_transition(field, trans_mlt, 0 , 1);
					mlt_transition_set_in_and_out(trans_mlt, 0 , mix_len - 1);
				}
			}
		}
	}

	if ( playlist == NULL ) {
		mlt_profile profile = mlt_profile_init(NULL);
		playlist = mlt_playlist_new(profile);
		playlist_wrap.obj = mlt_playlist_service(playlist);
	}

	json_t* props_je = json_object_get(js.h, "props");
	mlt_properties props = mlt_playlist_properties(playlist);

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
		playlist_wrap.obj = NULL;
		return mlt_playlist_service(playlist);
	}

	int aze = json_array_size(filters_ja);
	for ( int i=0; i<aze; i++ ) {
		json_t* jae = json_array_get(filters_ja, i);
		mlt_service mlt_obj = MltLoader::load_mlt(JsonWrap(jae));
		assert( filter_type == mlt_service_identify(mlt_obj));
		mlt_filter filter = MLT_FILTER(mlt_obj);
		mlt_producer_attach(mlt_playlist_producer(playlist), filter);
	}

#ifdef DEBUG

	std::cout << "======filter========" << json_string_value(json_object_get(js.h,"uuid"))<< "===filter====" <<endl;
	std::cout << mlt_playlist_properties(playlist);
	std::cout << "##############################################################" << endl;
#endif

	playlist_wrap.obj = NULL;
	return mlt_playlist_service(playlist);
}

NMSP_END(vedit)
