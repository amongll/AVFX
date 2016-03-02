/*
 * VEditVm.cpp
 *
 *  Created on: 2016年3月2日
 *      Author: li.lei@youku.com
 */


#include "VEditVM.h"

NMSP_BEGIN(vedit)

pthread_mutex_t Vm::script_lock = PTHREAD_MUTEX_INITIALIZER;

json_t* Vm::call_script(const char* procname, json_t* args) throw (Exception)
{
	shared_ptr<Script> script = get_script(procname);
	script->call(args);
	return script->get_mlt_serialize();
}

json_t* Vm::call_script(const char* procname, ScriptType type, json_t* args)
	throw (Exception)
{
	shared_ptr<Script> script = get_script(procname, type);
	script->call(args);
	return script->get_mlt_serialize();
}

shared_ptr<Script> Vm::get_script(const char* procname)
	throw (Exception)
{
	if (singleton.get() == NULL)
		throw Exception(ErrorImplError, "script %s not registed", procname);

	MapIter it;
	{
		Lock lk(&script_lock);
		it = singleton->all_defines.find(procname);
		if ( it == singleton->all_defines.end() ) {
			throw Exception(ErrorImplError, "script %s not registed", procname);
		}
	}

	Script* obj = NULL;
	switch (it->second.type) {
	case AUDIO_RESOURCE_SCRIPT:
		obj = new AudioScript(it->second.defines.h); //todo args
		break;
	case VIDEO_RESOURCE_SCRIPT:
		obj = new VideoScript(it->second.defines.h); //todo args
		break;
	case IMAGE_RESOURCE_SCRIPT:
		obj = new ImageScript(it->second.defines.h); //todo args
		break;
	case GIF_RESOURCE_SCRIPT:
		obj = new GifScript(it->second.defines.h); //todo args
		break;
	case FILTER_SCRIPT:
		obj = new FilterScript(it->second.defines.h); //todo args
		break;
	case PLAYLIST_SCRIPT:
		obj = new PlaylistScript(it->second.defines.h); //todo args
		break;
	case MULTITRACK_SCRIPT:
		obj = new MultitrackScript(it->second.defines.h); //todo args
		break;
	case TRANSITION_SCRIPT:
		obj = new TransitionScript(it->second.defines.h); //todo args
		break;
	default:
		assert(0);
		break;
	}
	return shared_ptr<Script>(obj);
}

pthread_key_t Vm::thr_spec_cache_key;
pthread_once_t Vm::thr_spec_cache_once = PTHREAD_ONCE_INIT;

void Vm::thr_spec_cache_key_create()
{
	pthread_key_create(&thr_spec_cache_key, Vm::thr_spec_cache_cleanup);
}

void Vm::thr_spec_cache_cleanup(void* dummy)
{
	StreamResourceCache* spec = static_cast<StreamResourceCache*>(dummy);
	pthread_setspecific(thr_spec_cache_key, NULL);
	delete spec;
}

Vm* Vm::singleton = NULL;
Vm* Vm::instance()
{
	pthread_once(&thr_spec_cache_once, Vm::thr_spec_cache_key_create);
	if ( !singleton ) {
		Lock(&script_lock);
		if (!singleton) {
			singleton = new Vm();
			singleton_ptr.reset(singleton);
			return singleton;
		}
		else{
			return singleton;
		}
	}
	else {
		return singleton;
	}
}

const char* Vm::proc_type_names[] = {
	"dummy",
	"audio",
	"video",
	"image",
	"gif",
	"filter",
	"playlist",
	"multitrack",
	"transition",
	NULL
};

void Vm::get_stream_resource_length(const string& path, int& in, int& out)
	throw (Exception)
{
	Vm::instance();
	StreamResourceCache* cache = static_cast<StreamResourceCache*>
		(pthread_getspecific(thr_spec_cache_key));
	if ( !cache ) {
		cache = new StreamResourceCache();
		pthread_setspecific(thr_spec_cache_key, cache);
	}

	hash_map<string, mlt_producer>::iterator it = cache->resources.find(path);
	mlt_producer prod = NULL;
	if ( it != cache->resources.end() ) {
		prod = it->second;
	}
	else {
		//todo: 选择默认的profile 仅fps和aspect_ratio影响producer
		string abs_path;
		get_absolute_path(path, abs_path);
		if (abs_path.size() == 0) {
			throw Exception(ErrorImplError, "resource path is invalid:%s", path.c_str());
		}

		mlt_profile profile = mlt_profile_init(NULL);
		mlt_producer prod = mlt_factory_producer(profile,"loader",(const void*)abs_path.c_str());
		if (prod == NULL)
			throw Exception(ErrorStreamFileInvalid,
					"audio/video stream file invalid:%s", path.c_str());

		cache->resources[path] = prod;
	}

	in = mlt_producer_get_in(prod);
	out = mlt_producer_get_out(prod);
	return;
}

shared_ptr<Script> Vm::get_script(const char* procname, ScriptType type)
	throw (Exception)
{
	MapIter it;
	{
		Lock lk(&script_lock);
		it = singleton->all_defines.find(procname);
		if ( it == singleton->all_defines.end() ) {
			throw Exception(ErrorImplError, "script %s not registed", procname);
		}
	}

	if ( it->second.type != type ) {
		throw Exception(ErrorImplError, "script type mismatch", procname);
	}

	return get_script(procname);
}

void Vm::regist_script(json_t* text)throw(Exception)
{
	Vm* vm = instance();
	if ( !text || !json_is_object(text) || !json_object_size(text) ) {
		throw Exception(ErrorScriptFmtError, "script text invalid. ");
	}

	json_t* se = json_object_get(text, "proctype");
	if ( !se || !json_is_string(se) || !strlen(json_string_value(se))) {
		throw Exception(ErrorScriptFmtError, "script text invalid. proctype required");
	}

	ScriptType type = INVALID_SCRIPT;
	for ( int i=1; i<INVALID_SCRIPT; i++) {
		if (!strcmp(json_string_value(se), proc_type_names[i]) == 0) {
			type = i;
			break;
		}
	}

	if ( type == INVALID_SCRIPT ) {
		throw Exception(ErrorScriptFmtError, "script text invalid. proctype:%s not implemented",
				json_string_value(se));
	}

	Script* obj = NULL;
	switch (type) {
	case AUDIO_RESOURCE_SCRIPT:
		obj = new AudioScript(text); //todo args
		break;
	case VIDEO_RESOURCE_SCRIPT:
		obj = new VideoScript(text); //todo args
		break;
	case IMAGE_RESOURCE_SCRIPT:
		obj = new ImageScript(text); //todo args
		break;
	case GIF_RESOURCE_SCRIPT:
		obj = new GifScript(text); //todo args
		break;
	case FILTER_SCRIPT:
		obj = new FilterScript(text); //todo args
		break;
	case PLAYLIST_SCRIPT:
		obj = new PlaylistScript(text); //todo args
		break;
	case MULTITRACK_SCRIPT:
		obj = new MultitrackScript(text); //todo args
		break;
	case TRANSITION_SCRIPT:
		obj = new TransitionScript(text); //todo args
		break;
	default:
		assert(0);
		break;
	}

	Lock lk(&script_lock);
	Wrap& wrap = vm->all_defines[obj->proc_name];
	wrap.type = type;
	wrap.defines.h = json_deep_copy(text);
	delete obj;
}

void Vm::regist_script(int fd)throw(Exception)
{
	FILE* fp = fdopen(fd, "r");
	try {
		regist_script(fp);
	}
	catch(const Exception& e) {
		fclose(fp);
		throw;
	}
	fclose(fp);
}

void Vm::regist_script(FILE* fp)throw(Exception)
{
	json_error_t err;
	json_t* text = json_loadf(fp, 0 ,&err);
	if (!text) {
		throw Exception(ErrorScriptFmtError, "script json error:%s at line:%d", err.text, err.line);
	}
	try {
		regist_script(text);
	}
	catch(const Exception& e) {
		json_decref(text);
		throw;
	}
	json_decref(text);
}

Vm::StreamResourceCache::~StreamResourceCache()
{
	hash_map<string, mlt_producer>::iterator it;
	for (it = resources.begin(); it != resources.end(); it++ ){
		mlt_producer_close(it->second);
	}
}



NMSP_END(vedit)

