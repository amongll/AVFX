/*
 * VEditPlaylistScript.cpp
 *
 *  Created on: 2016年3月7日
 *      Author: li.lei@youku.com
 */

#include "VEditPlaylistScript.h"

NMSP_BEGIN(vedit)

PlaylistScript::~PlaylistScript()
{
}

json_t* PlaylistScript::compile() throw (Exception)
{
	if (slices.size() == 0) return NULL;
	SliceIter it;
	json_t* ja  = NULL;
	for ( it = slices.begin(); it != slices.end(); it++ ) {
		json_t* ae = NULL;
		if ( it->call && it->call_result.second.h ) {
			if (!ae) ae = json_object();

			const char* k = NULL;
			json_t* je = NULL;
			json_object_foreach(it->call_result.second.h, k, je)
			{
				json_object_set(ae, k, je);
			}
		}

		if ( it->ctrl_result.h ) {
			if (!ae) ae = json_object();
			json_object_set(ae, "playlist_slice_ctrl", it->ctrl_result.h);
		}

		if (ae) {
			if (!ja) ja = json_array();
			json_array_append(ja, ae);
		}
	}

	if ( ja ) {
		json_t* ret = json_object();
		json_object_set_new(ret, "slices", ja);
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

	slices.resize(asz);
	for ( int i=0; i< asz; i++ ) {
		json_t* ae = json_array_get(slices_ja, i);
		json_t* call_je = NULL, *ctrl_je = NULL;
		void* it = json_object_iter(ae);
		while(it) {
			const char* k = json_object_iter_key(it);
			json_t* aee = json_object_iter_value(it);
			it = json_object_iter_next(ae, it);

			if ( !strncasecmp(k, "$call(", strlen("$call("))) {
				call_je = json_object();
				json_object_set(call_je, k, aee);
			}
			else if ( !strcasecmp( k, "ctrl") ) {
				ctrl_je = aee;
			}
		}
		JsonWrap call_je_wrap(call_je);
		ScriptCallable* call_obj = NULL;
		ScriptProps* ctrl_props = NULL;

		if ( call_je ) {
			call_obj = new ScriptCallable(*this, call_je);
			slices[i].call.reset(call_obj);
		}
		if (ctrl_je) {
			vector<string> specs ;
			specs.push_back("blank_frames");
			specs.push_back("blank_time");
			specs.push_back("repeat");
			specs.push_back("mix_frames");
			ctrl_props = new ScriptProps(*this, ctrl_je, specs);
			slices[i].ctrls.reset(ctrl_props);
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
	}

	/**
	 * 临时组件一个playlist对象，来决定playlist的持续时长。
	 * 组成playlist的slice和transtion对象会被临时注册。
	 */
	mlt_playlist playlist = NULL;
	MltSvcWrap playlist_wrap;
	mlt_transition pending_transition = NULL;
	int pending_mix_len = 0;
	for ( it = slices.begin(); it != slices.end(); it++ ) {
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
			case transition_type:
				break;
			default:
				throw_error_v(ErrorScriptFmtError, "Playlist script slice call type error");
			}
		}

		if ( svc_obj == NULL ) {
			if ( it->ctrl_result.h == NULL)
				continue;

			bool timeflag = false;
			json_t* blk_je = json_object_get(it->ctrl_result.h, "blank_frames");
			if ( blk_je == NULL || !json_is_integer(blk_je) ) {
				blk_je = json_object_get(it->ctrl_result.h, "blank_time");
				if ( blk_je && !json_is_integer(blk_je))
					blk_je = NULL;
				if (blk_je) timeflag = true;
			}
			if (blk_je == NULL)
				continue;

			int blkv = json_integer_value(blk_je);
			if (blkv <= 0) {
				throw_error_v(ErrorScriptFmtError, "playlist blank slice should be positive duration");
			}
			if (timeflag) blkv /= 40;

			if (playlist == NULL) {
				mlt_profile profile = mlt_profile_init(NULL);
				playlist = mlt_playlist_new(profile);
				playlist_wrap.obj = mlt_playlist_service(playlist);
			}

			mlt_playlist_blank(playlist, blkv);
		}
		else {
			if (type == transition_type) {
				if ( it + 1 == slices.end() || it == slices.begin() ) {
					throw_error_v(ErrorScriptFmtError, "playlist mix must be between slices");
				}

				if ( it->ctrl_result.h == NULL  ) {
					throw_error_v(ErrorScriptFmtError, "playlist slice mix require mix_frames ctrl");
				}

				json_t* mlje = json_object_get(it->ctrl_result.h, "mix_frames");
				if ( !mlje || !json_is_integer(mlje) ) {
					throw_error_v(ErrorScriptFmtError, "playlist slice mix require mix_frames ctrl");
				}

				int mix_len = json_integer_value(mlje);
				if (mix_len <= 0 ) {
					mlt_transition_close(MLT_TRANSITION(svc_obj));
					throw_error_v(ErrorScriptFmtError, "playlist slice mix require mix_frames ctrl");
				}

				pending_transition = MLT_TRANSITION(svc_obj);
				pending_mix_len = mix_len;
				MltLoader::push_mlt_registry(svc_obj, it->call_result.first);
			}
			else {
				if (playlist == NULL) {
					mlt_profile profile = mlt_profile_init(NULL);
					playlist = mlt_playlist_new(profile);
					playlist_wrap.obj = mlt_playlist_service(playlist);
				}

				mlt_playlist_append(playlist, MLT_PRODUCER(svc_obj));
				if (pending_transition) {
					int slice_idx = mlt_playlist_count(playlist) - 2;
					mlt_playlist_mix(playlist, slice_idx, pending_mix_len, pending_transition);
					pending_transition = NULL;
				}
				MltLoader::push_mlt_registry(svc_obj, it->call_result.first);
			}
		}
	}

	if (playlist == NULL) {
		set_frame_range(0,0);
		return;
	}

	set_frame_range(0, mlt_producer_get_out(mlt_playlist_producer(playlist)));
	return;
}

void PlaylistLoader::declare()
{
	MltLoader::regist_loader("playlist", &PlaylistLoader::load_playlist);
}

mlt_service PlaylistLoader::load_playlist(JsonWrap js) throw (Exception)
{
	json_t* slices_ja = json_object_get(js.h, "slices");
	mlt_playlist playlist = NULL;
	json_t* ctrl_je = NULL;
	mlt_producer slice_mlt = NULL;
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

		ctrl_je = json_object_get(mlt_e, "playlist_slice_ctrl");
		if (ctrl_je) {
			json_incref(ctrl_je);
			json_object_del(mlt_e, "playlist_slice_ctrl");
		}
		JsonWrap ctrlwrap(ctrl_je, 1);

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
			case transition_type:
				mix = MLT_TRANSITION(mlt_obj);
				break;
			default:
				slice_mlt = MLT_PRODUCER(mlt_obj);
				break;
			}
		}

		if (slice_mlt) {
			if (playlist==NULL) {
				mlt_profile profile = mlt_profile_init(NULL);
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

			if (mix) {
				assert(ctrl_je);
				json_t* mix_len_je = json_object_get(ctrl_je, "mix_frames");
				assert(mix_len_je && json_is_integer(mix_len_je) && json_integer_value(mix_len_je)>0);
				int idx = mlt_playlist_count(playlist) - 2;
				mlt_playlist_mix(playlist, idx, json_integer_value(mix_len_je), mix);
				mix_wrap.obj = NULL;
				mix = NULL;
			}
		}
		else if (mix) {
			assert( i > 0 && i < slices_sz - 1); //pending this
			mix_wrap.obj = mlt_obj;
		}
		else if (ctrl_je) {
			//blank

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

			if (timeflag) blk = blk/40;

			mlt_playlist_blank(playlist, blk);
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

	playlist_wrap.obj = NULL;
	return mlt_playlist_service(playlist);
}

NMSP_END(vedit)
