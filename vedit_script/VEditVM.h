/*
 * VEditVM.h
 *
 *  Created on: 2016年2月22日
 *      Author: li.lei@youku.com
 */

#ifndef VEDITVM_H_
#define VEDITVM_H_

#include "VEditCommon.h"
#include "VEditProducerScript.h"
#include "VEditFilterScript.h"
#include "VEditPlaylistScript.h"
#include "VEditMultiTrackScript.h"
#include "VEditTransitionScript.h"

NMSP_BEGIN(vedit)

class Vm
{
public:
	enum ScriptType {
		UNKNOWN_SCRIPT,
		AUDIO_RESOURCE_SCRIPT,
		VIDEO_RESOURCE_SCRIPT,
		IMAGE_RESOURCE_SCRIPT,
		GIF_RESOURCE_SCRIPT,
		FILTER_SCRIPT,
		PLAYLIST_SCRIPT,
		MULTITRACK_SCRIPT,
		TRANSITION_SCRIPT,
		INVALID_SCRIPT //占位符
	};

	static json_t* call_script(const char* procname, json_t* args) throw(Exception);
	static json_t* call_script(const char* procname, ScriptType type, json_t* args)throw(Exception);
	static shared_ptr<Script> get_script(const char* procname)throw(Exception);

	static shared_ptr<VideoScript> get_video_script(const char* procname)
	{
		return shared_ptr<VideoScript>(dynamic_cast<VideoScript*>
		( get_script(procname, VIDEO_RESOURCE_SCRIPT).get()) );
	}

	static shared_ptr<AudioScript> get_audio_script(const char* procname)
	{
		return shared_ptr<AudioScript>(dynamic_cast<AudioScript*>
		( get_script(procname, AUDIO_RESOURCE_SCRIPT).get()) );
	}

	static shared_ptr<ImageScript> get_image_script(const char* procname)
	{
		return shared_ptr<ImageScript>(dynamic_cast<ImageScript*>
		( get_script(procname, IMAGE_RESOURCE_SCRIPT).get()) );
	}

	static shared_ptr<GifScript> get_gif_script(const char* procname)
	{
		return shared_ptr<GifScript>(dynamic_cast<GifScript*>
		( get_script(procname, GIF_RESOURCE_SCRIPT).get()) );
	}

	static shared_ptr<FilterScript> get_filter_script(const char* procname)
	{
		return shared_ptr<FilterScript>(dynamic_cast<FilterScript*>
		( get_script(procname, FILTER_SCRIPT).get()) );
	}

	static shared_ptr<PlaylistScript> get_playlist_script(const char* procname)
	{
		return shared_ptr<PlaylistScript>(dynamic_cast<PlaylistScript*>
		( get_script(procname, PLAYLIST_SCRIPT).get()) );
	}

	static shared_ptr<MultitrackScript> get_multitrack_script(const char* procname)
	{
		return shared_ptr<MultitrackScript>(dynamic_cast<MultitrackScript*>
		( get_script(procname, MULTITRACK_SCRIPT).get()) );
	}

	static shared_ptr<TransitionScript> get_transition_script(const char* procname)
	{
		return shared_ptr<TransitionScript>(dynamic_cast<TransitionScript*>
		( get_script(procname, TRANSITION_SCRIPT).get()) );
	}

	static void regist_script(json_t* text)throw(Exception);
	static void regist_script(int fd)throw(Exception);
	static void regist_script(FILE* fp)throw(Exception);

	static void get_stream_resource_length(const string& path, int& in, int& out) throw(Exception);

private:
	Vm(){};
	static shared_ptr<Vm> singleton_ptr;
	static Vm* singleton;
	static Vm* instance();
	static pthread_mutex_t script_lock;

	static pthread_key_t thr_spec_cache_key;
	static pthread_once_t thr_spec_cache_once;
	static void thr_spec_cache_cleanup(void*);
	static void thr_spec_cache_key_create();

	static shared_ptr<Script> get_script(const char* procnmae, ScriptType type) throw (Exception);
	static const char* proc_type_names[ INVALID_SCRIPT ];

	//线程转悠对象
	struct StreamResourceCache
	{
		~StreamResourceCache();
		hash_map<string, mlt_producer> resources;
	};

	struct Wrap {
		Wrap(ScriptType type_arg = UNKNOWN_SCRIPT, json_t* in = NULL):
			type(type_arg),
			defines(in)
		{}

		ScriptType type;
		JsonWrap defines;
	};

	typedef hash_map<string, Wrap>::iterator MapIter;
	hash_map<string, Wrap> all_defines;
};

NMSP_END(vedit)


#endif /* VEDITVM_H_ */
