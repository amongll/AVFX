/*
 * VEditVm.cpp
 *
 *  Created on: 2016��3��2��
 *      Author: li.lei@youku.com
 */


#include "VEditVM.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>
#include <framework/mlt.h>
#include <sys/time.h>
#include <time.h>

#include <dirent.h>

NMSP_BEGIN(vedit)

pthread_mutex_t Vm::script_lock = PTHREAD_MUTEX_INITIALIZER;
shared_ptr<Vm> Vm::singleton_ptr;
unsigned int Vm::rand_seed = time(NULL);

ScriptSerialized Vm::call_script(const char* procname, json_t* args) throw (Exception)
{
	shared_ptr<Script> script = get_script(procname);
	script->call(args);
	return script->get_mlt_serialize();
}

ScriptSerialized Vm::call_script(const char* procname, vedit::ScriptType type, json_t* args)
	throw (Exception)
{
	shared_ptr<Script> script(get_script(procname, type));
	script->call(args);
	return script->get_mlt_serialize();
}

shared_ptr<Script> Vm::get_script(const char* procname)
	throw (Exception)
{
	return shared_ptr<Script>(get_script_impl(procname));
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

Vm* Vm::singleton = Vm::instance();
Vm* Vm::instance()
{
	pthread_once(&thr_spec_cache_once, Vm::thr_spec_cache_key_create);
	if ( !singleton ) {
		Lock lk(&script_lock);
		if (!singleton) {
			SingleResourceLoader::declare();
			FilterLoader::declare();
			PlaylistLoader::declare();
			TransitionLoader::declare();
			MultitrackLoader::declare();
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
	"transition"
};

void Vm::load_script_dir(const char* path) throw (Exception)
{
	string inpath(path), dir_abs;
	get_absolute_path(inpath, dir_abs);
	struct stat stbuf;
	if ( -1 == stat(dir_abs.c_str(),&stbuf) ) {
		throw_error_v(ErrorFileSysError, "dir %s not exists", dir_abs.c_str());
	}

	if ( !S_ISDIR(stbuf.st_mode) ) {
		throw_error_v(ErrorFileSysError, "%s not dir", dir_abs.c_str());
	}

	DIR* diobj = opendir(dir_abs.c_str());
	if (diobj == NULL) {
		throw_error_v(ErrorFileSysError, "%s scan failed:%s", dir_abs.c_str(), strerror(errno));
	}

	struct dirent *de = NULL;
	Exception eCopy;
	char tmp_path[1024];
	while ((de = readdir(diobj))) {
		if (strlen(de->d_name) <= strlen(".json"))
			continue;
		if (strcmp(".json", de->d_name + strlen(de->d_name) - strlen(".json"))
			!= 0)
			continue;
		snprintf(tmp_path, sizeof(tmp_path), "%s/%s", dir_abs.c_str(),
			de->d_name);
		if (-1 == stat(tmp_path, &stbuf))
			continue;

		if (!S_ISREG(stbuf.st_mode))
			continue;

		if ( access(tmp_path, R_OK) ) {
			continue;
		}

		FILE* fp = fopen(tmp_path, "r");
		if (!fp) continue;
		try {
			regist_script(fp);
		}
		catch(const Exception& e) {
			fclose(fp);
			mlt_log_warning(NULL, "vedit_script:%s", e.what());
		}
	}

	closedir(diobj);
}

Script* Vm::get_script_impl(const char* procname) throw (Exception)
{
	if (singleton == NULL)
		throw_error_v(ErrorImplError, "script %s not registed", procname);

	MapIter it;
	{
		Lock lk(&script_lock);
		it = singleton->all_defines.find(procname);
		if ( it == singleton->all_defines.end() ) {
			throw_error_v(ErrorImplError, "script %s not registed", procname);
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
	return obj;
}

Script* Vm::get_script(const char* procname, ScriptType type)
	throw (Exception)
{
	MapIter it;
	{
		Lock lk(&script_lock);
		it = singleton->all_defines.find(procname);
		if ( it == singleton->all_defines.end() ) {
			throw_error_v(ErrorImplError, "script %s not registed", procname);
		}
	}

	if ( it->second.type != type ) {
		throw_error_v(ErrorImplError, "script type mismatch", procname);
	}

	return get_script_impl(procname);
}

void Vm::regist_script(json_t* text)throw(Exception)
{
	Vm* vm = instance();
	if ( !text || !json_is_object(text) || !json_object_size(text) ) {
		throw_error_v(ErrorScriptFmtError, "script text invalid. ");
	}

	json_t* se = json_object_get(text, "proctype");
	if ( !se || !json_is_string(se) || !strlen(json_string_value(se))) {
		throw_error_v(ErrorScriptFmtError, "script text invalid. proctype required");
	}

	ScriptType type = INVALID_SCRIPT;
	for ( int i=1; i<INVALID_SCRIPT; i++) {
		if (strcmp(json_string_value(se), proc_type_names[i]) == 0) {
			type = (ScriptType)i;
			break;
		}
	}

	if ( type == INVALID_SCRIPT ) {
		throw_error_v(ErrorScriptFmtError, "script text invalid. proctype:%s not implemented",
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
		throw_error_v(ErrorScriptFmtError, "script json error:%s at line:%d", err.text, err.line);
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

#include <endian.h>

string Vm::uuid()
{
	static uint16_t local_rand1 = (uint16_t)rand_r(&rand_seed)%0x10000;
	static uint16_t local_rand2 = (uint16_t)rand_r(&rand_seed)%0x10000;
	static int local_pid = htobe32(getpid());
	Lock lk(&script_lock);
	struct timeval tv;
	gettimeofday(&tv, NULL);

	uint64_t v = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	v *= 65536;
	v += rand_r(&rand_seed)%65535;

	v = htobe64(v);

	char buf[37];
	char *p = buf;

	uint8_t* pv = (uint8_t*)&v;
	for ( int i=0; i<8; i++ ) {
		p += snprintf(p, 3, "%02x", *pv++);
		switch(i) {
		case 3:
		case 5:
		case 7:
			*p++ = '-';
			break;
		}
	}

	p += snprintf(p, 6, "%04x-", local_rand1);
	p += snprintf(p, 5, "%04x", local_rand2);

	pv = (uint8_t*)&local_pid;
	for ( int i=0; i<4; i++) {
		p += snprintf(p, 3, "%02x", *pv++);
	}

	return string(buf);
}


NMSP_END(vedit)


