/*
 * VEditVM.h
 *
 *  Created on: 2016��2��22��
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

#ifdef __ANDROID__
#include <android/asset_manager.h>
#endif

NMSP_BEGIN(vedit)

class Vm
{
public:
	static ScriptSerialized call_script(const char* procname, json_t* args) throw(Exception);
	static ScriptSerialized call_script(const char* procname, vedit::ScriptType type, json_t* args)throw(Exception);
	static std::tr1::shared_ptr<Script> get_script(const char* procname)throw(Exception);

	static string uuid();

	static std::tr1::shared_ptr<VideoScript> get_video_script(const char* procname)
	{
		return std::tr1::shared_ptr<VideoScript>(dynamic_cast<VideoScript*>
		( get_script(procname, VIDEO_RESOURCE_SCRIPT)) );
	}

	static std::tr1::shared_ptr<AudioScript> get_audio_script(const char* procname)
	{
		return std::tr1::shared_ptr<AudioScript>(dynamic_cast<AudioScript*>
		( get_script(procname, AUDIO_RESOURCE_SCRIPT)) );
	}

	static std::tr1::shared_ptr<ImageScript> get_image_script(const char* procname)
	{
		return std::tr1::shared_ptr<ImageScript>(dynamic_cast<ImageScript*>
		( get_script(procname, IMAGE_RESOURCE_SCRIPT)) );
	}

	static std::tr1::shared_ptr<GifScript> get_gif_script(const char* procname)
	{
		return std::tr1::shared_ptr<GifScript>(dynamic_cast<GifScript*>
		( get_script(procname, GIF_RESOURCE_SCRIPT)) );
	}

	static std::tr1::shared_ptr<FilterScript> get_filter_script(const char* procname)
	{
		return std::tr1::shared_ptr<FilterScript>(dynamic_cast<FilterScript*>
		( get_script(procname, FILTER_SCRIPT)) );
	}

	static std::tr1::shared_ptr<PlaylistScript> get_playlist_script(const char* procname)
	{
		return std::tr1::shared_ptr<PlaylistScript>(dynamic_cast<PlaylistScript*>
		( get_script(procname, PLAYLIST_SCRIPT)) );
	}

	static std::tr1::shared_ptr<MultitrackScript> get_multitrack_script(const char* procname)
	{
		return std::tr1::shared_ptr<MultitrackScript>(dynamic_cast<MultitrackScript*>
		( get_script(procname, MULTITRACK_SCRIPT)) );
	}

	static std::tr1::shared_ptr<TransitionScript> get_transition_script(const char* procname)
	{
		return std::tr1::shared_ptr<TransitionScript>(dynamic_cast<TransitionScript*>
		( get_script(procname, TRANSITION_SCRIPT)) );
	}

	static std::tr1::shared_ptr<AsisScript> get_asis_producer_script(const char* procname)
	{
		return std::tr1::shared_ptr<AsisScript>(dynamic_cast<AsisScript*>
		( get_script(procname, ASIS_PRODUCER_SCRIPT)) );
	}

	static void regist_script(json_t* text)throw(Exception);
	static void regist_script(int fd)throw(Exception);
	static void regist_script(FILE* fp)throw(Exception);

	static void load_script_dir(const char* path) throw (Exception);
#ifdef __ANDROID__
	static void load_script_assets_dir(AAssetManager *amgr, const char* path) throw(Exception);

	static void android_init(AAssetManager* amgr,
		const char* files_root, const vector<string>& plugins) throw(Exception);

	static void android_log_init(const char* logtag, int loglevel);
#endif

	static void init(const char* profilenm = NULL) throw (Exception);
	static const char* proc_type_names[ INVALID_SCRIPT ];
private:
	Vm(){};
	static std::tr1::shared_ptr<Vm> singleton_ptr;
	static Vm* singleton;
	static Vm* instance();
	static pthread_mutex_t script_lock;

	static unsigned int rand_seed;

	static pthread_key_t thr_spec_cache_key;
	static pthread_once_t thr_spec_cache_once;
	static void thr_spec_cache_cleanup(void*);
	static void thr_spec_cache_key_create();

	static Script* get_script(const char* procnmae, ScriptType type) throw (Exception);
	static Script* get_script_impl(const char* procname) throw (Exception);

#ifdef __ANDROID__
	static void logCallback(void* ptr, int level, const char* fmt, va_list vl) ;



#endif

	struct MltRepoWrap
	{
		mlt_repository repo;
		MltRepoWrap(mlt_repository in=NULL):repo(in){}
		~MltRepoWrap(){
			if ( repo ) mlt_factory_close2();
		}
	};

	static MltRepoWrap mltFactory;

	static int mltLogLevel;
	static string mltLogTag;

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
