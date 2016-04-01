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

#ifdef __ANDROID__
#include <android/log.h>
#endif

NMSP_BEGIN(vedit)

pthread_mutex_t Vm::script_lock = PTHREAD_MUTEX_INITIALIZER;
std::tr1::shared_ptr<Vm> Vm::singleton_ptr;
unsigned int Vm::rand_seed = time(NULL);

ScriptSerialized Vm::call_script(const char* procname, json_t* args) throw (Exception)
{
	std::tr1::shared_ptr<Script> script = get_script(procname);
	script->call(args);
	return script->get_mlt_serialize();
}

ScriptSerialized Vm::call_script(const char* procname, vedit::ScriptType type, json_t* args)
	throw (Exception)
{
	std::tr1::shared_ptr<Script> script(get_script(procname, type));
	script->call(args);
	return script->get_mlt_serialize();
}

std::tr1::shared_ptr<Script> Vm::get_script(const char* procname)
	throw (Exception)
{
	return std::tr1::shared_ptr<Script>(get_script_impl(procname));
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
			srand(rand_seed);
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
	"transition",
	"asis_producer"
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
			fp = NULL;
			mlt_log_warning(NULL, "vedit_script:%s", e.what());
		}
		fclose(fp);
	}

	closedir(diobj);
}

int Vm::mltLogLevel = MLT_LOG_INFO;
string Vm::mltLogTag="vedit_script";

Vm::MltRepoWrap Vm::mltFactory;


#ifdef __ANDROID__

void Vm::android_init(AAssetManager* amgr,
		const char* files_root, const vector<string>& plugins) throw(Exception)
{
	if ( !files_root || !strlen(files_root) || !amgr ||  plugins.size()==0)
		throw_error_v(ErrorImplError, "vedit_script init failed");

	char tmp[1024];
	snprintf(tmp,sizeof(tmp),"%s/mlt",files_root);
	setenv("MLT_DATA", tmp, 1);

	if (!mlt_android_quick_copy_AAssets(amgr, files_root, tmp, sizeof(tmp)) ) {
		throw_error_v(ErrorImplError, "vedit_script MLT ENGINE presets problem.");
	}

	const char* __plugins[256] = {NULL};
	vector<string>::const_iterator it;
	const char** p = __plugins;
	for ( it = plugins.begin(); it!= plugins.end() ;it++, p++) {
		*p = it->c_str();
	}
	mltFactory.repo = mlt_factory_init2(__plugins, p - __plugins);
	if (mltFactory.repo == NULL) {
		throw_error_v(ErrorImplError, "MLT ENGINE factory init failed.");
	}
}

void Vm::android_log_init(const char* logtag, int loglevel)
{
	switch (loglevel) {
	case ANDROID_LOG_VERBOSE:
		mltLogLevel = MLT_LOG_INFO;
		break;
	case ANDROID_LOG_DEBUG:
		mltLogLevel = MLT_LOG_INFO;
		break;
	case ANDROID_LOG_INFO:
		mltLogLevel = MLT_LOG_INFO;
		break;
	case ANDROID_LOG_WARN:
		mltLogLevel = MLT_LOG_WARNING;
		break;
	case ANDROID_LOG_ERROR:
		mltLogLevel = MLT_LOG_ERROR;
		break;
	case ANDROID_LOG_FATAL:
		mltLogLevel = MLT_LOG_FATAL;
		break;
	}
	if (logtag && strlen(logtag))
		mltLogTag = logtag;

	LOGTAG = mltLogTag.c_str();

	mlt_log_set_level(mltLogLevel);
	mlt_log_set_callback(logCallback);
 }

void Vm::logCallback(void* ptr, int level, const char* fmt, va_list vl)
{
	int print_prefix = 1;
	mlt_properties properties = ptr ? MLT_SERVICE_PROPERTIES( ( mlt_service )ptr ) : NULL;
	char prefixbuf[512] = "";
	char logbuf[1024] = "";

	static const char* prefix_fmt1= "[%s %s] %s\n";
	static const char* prefix_fmt2= "[%s %p] %s\n";

	if ( level > mltLogLevel ) {
		return;
	}
	if ( print_prefix && properties )
	{
		char *mlt_type = mlt_properties_get( properties, "mlt_type" );
		char *mlt_service = mlt_properties_get( properties, "mlt_service" );
		char *resource = mlt_properties_get( properties, "resource" );

		if ( !( resource && *resource && resource[0] == '<' && resource[ strlen(resource) - 1 ] == '>' ) )
			mlt_type = mlt_properties_get( properties, "mlt_type" );
		if (mlt_service) {
			snprintf(prefixbuf, sizeof(prefixbuf), prefix_fmt1, mlt_type, mlt_service, resource);
		}
		else {
			snprintf(prefixbuf, sizeof(prefixbuf), prefix_fmt2, mlt_type, ptr, resource);
		}
	}
	print_prefix = strstr( fmt, "\n" ) != NULL;
	vsnprintf( logbuf,sizeof(logbuf), fmt, vl );

	int aLevel = ANDROID_LOG_INFO;
	switch(level)
	{
	case MLT_LOG_DEBUG:
	case MLT_LOG_VERBOSE:
		aLevel = ANDROID_LOG_DEBUG;
		break;
	case MLT_LOG_INFO:
		aLevel = ANDROID_LOG_INFO;
		break;
	case MLT_LOG_WARNING:
		aLevel = ANDROID_LOG_WARN;
		break;
	case MLT_LOG_ERROR:
		aLevel = ANDROID_LOG_ERROR;
		break;
	case MLT_LOG_FATAL:
		aLevel = ANDROID_LOG_FATAL;
		break;
	default:
		break;
	}

	if (print_prefix) {
		__android_log_print(aLevel, mltLogTag.c_str(), "%s%s", prefixbuf, logbuf);
	}
	else {
		__android_log_print(aLevel, mltLogTag.c_str(), "%s", logbuf);
	}
}

void Vm::load_script_assets_dir(AAssetManager *amgr, const char* path) throw(Exception)
{
	if ( amgr == NULL || path == NULL ) {
		return;
	}

	AAssetDir* adir = AAssetManager_openDir(amgr, path);
	if (adir == NULL) {
		throw_error_v(ErrorInvalidParam, "open assets dir:%s failed", path);
	}

	const char* filenm = NULL;
	while (( filenm = AAssetDir_getNextFileName(adir) ) ) {
		if (strlen(filenm) <= strlen(".json"))
			continue;
		if (strcmp(".json", filenm + strlen(filenm) - strlen(".json"))
			!= 0)
			continue;

		AAsset* aset = AAssetManager_open(amgr, filenm, O_RDONLY);
		if (!aset) continue;
		off_t aset_size = AAsset_getLength(aset);
		if (aset_size == 0) continue;
		char * buf = new char[aset_size+1];
		AAsset_read(aset, buf, aset_size);
		buf[aset_size]=0;
		AAsset_close(aset);

		json_error_t jserr;
		json_t* jsobj = json_loads(buf, 0, &jserr);
		free(buf);

		if (!jsobj) {
			ALOGW("parse json failed when load asset template:%s, cause: %d:%d at<%s> %s", filenm,
				jserr.line, jserr.position, jserr.source, jserr.text);
			continue;
		}

		JsonWrap jswrp(jsobj, 1);

		try {
			regist_script(jsobj);
		}
		catch(const Exception& e) {
			ALOGW("load asset template:%s failed, cause:%s", filenm, e.what());
		}
	}
	AAssetDir_close(adir);
}
#endif

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
	case ASIS_PRODUCER_SCRIPT:
		obj = new AsisScript(it->second.defines.h); //todo args
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
	case ASIS_PRODUCER_SCRIPT:
		obj = new AsisScript(text); //todo args
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
	static uint16_t local_rand1 = (uint16_t)rand()%0x10000;
	static uint16_t local_rand2 = (uint16_t)rand()%0x10000;
	static int local_pid = htobe32(getpid());
	Lock lk(&script_lock);
	struct timeval tv;
	gettimeofday(&tv, NULL);

	uint64_t v = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	v *= 65536;
	v += rand()%65535;

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

void Vm::init(const char* profilenm) throw (Exception)
{
	instance();
	MltLoader::global_profile = mlt_profile_init(profilenm);
}

NMSP_END(vedit)


