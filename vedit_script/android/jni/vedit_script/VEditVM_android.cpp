/*
 * VEditVM_android.cpp
 *
 *  Created on: 2016年3月29日
 *      Author: li.lei@youku.com
 */

#ifdef __ANDROID__
#include "framework/mlt.h"
#include "VEditVM.h"
#include <android/asset_manager_jni.h>

NMSP_BEGIN(vedit)

#define CLASSNAME "com/youku/cloud/vedit/MltTemplate"

static struct JNIJavaHook
{
	jclass clazz;
	jclass java_io_file_clazz;
	jmethodID java_io_file_getPath_method;
	jclass java_string_clazz;
	string last_error;
}gHook;

static string GetPathFromJFile(JNIEnv* env, jobject jfile, string& fail_desc)
{
	if ( jfile == NULL) {
		fail_desc = "null object";
		return "";
	}

	jstring path_jstr = (jstring)env->CallObjectMethod(jfile, gHook.java_io_file_getPath_method);
	jboolean copyio = false;
	const char* path_cstr = env->GetStringUTFChars(path_jstr, &copyio);
	string path_str(path_cstr);
	env->ReleaseStringUTFChars(path_jstr, path_cstr);
	env->DeleteLocalRef(path_jstr);

	return path_str;
}

static json_t* LoadJsonFromJFile(JNIEnv* env, jobject jfile, string& fail_desc)
{
	if ( jfile == NULL ) {
		fail_desc = "null object";
		return NULL;
	}

	jstring path_jstr = (jstring)env->CallObjectMethod(jfile, gHook.java_io_file_getPath_method);
	jboolean copyio = false;
	const char* path_cstr = env->GetStringUTFChars(path_jstr, &copyio);
	string path_str(path_cstr);
	env->ReleaseStringUTFChars(path_jstr, path_cstr);
	env->DeleteLocalRef(path_jstr);

	json_error_t js_err;
	json_t* js_obj = json_load_file(path_str.c_str(), 0, &js_err);
	if ( js_obj == NULL ) {
		char buf[1024];
		snprintf(buf, sizeof(buf),"load json file failed:%s at: %d:%d %s. cause:%s",
				path_str.c_str(), js_err.line, js_err.position,js_err.source, js_err.text);
		ALOGI("%s",buf);
		fail_desc = buf;
	}
	return js_obj;
}

static json_t* LoadJsonFromJString(JNIEnv* env, jstring jstr, string& fail_desc)
{
	if (jstr == NULL) {
		fail_desc = "null object";
		return NULL;
	}
	jboolean copyio = false;
	const char* path_cstr = env->GetStringUTFChars(jstr, &copyio);
	string path_str(path_cstr);
	env->ReleaseStringUTFChars(jstr, path_cstr);
	json_error_t js_err;
	json_t* js_obj = json_load_file(path_str.c_str(), 0, &js_err);
	if ( js_obj == NULL ) {
		char buf[1024];
		snprintf(buf, sizeof(buf),"load json file failed:%s at: %d:%d %s. cause:%s",
				path_str.c_str(), js_err.line, js_err.position,js_err.source, js_err.text);
		ALOGI("%s",buf);
		fail_desc = buf;
	}
	return js_obj;
}

#define GET_CPPSTR_FROM_JSTR(env__,jstr__,cppstr__) do{\
	jboolean copyio=false;\
	const char* cstr = (env__)->GetStringUTFChars((jstr__),&copyio);\
	(cppstr__) = cstr;\
	(env__)->ReleaseStringUTFChars((jstr__), cstr);\
}while(0)

static jboolean Init(JNIEnv* env, jclass clzz, jstring profile_name, jstring logtag, jint loglevel)
{
	string pnm;
	GET_CPPSTR_FROM_JSTR(env, logtag, pnm);
	Vm::android_log_init(pnm.c_str(), loglevel);

	GET_CPPSTR_FROM_JSTR(env, profile_name, pnm);
	const char* cstr = pnm.size()>0?pnm.c_str():NULL;
	Vm::init(cstr);

	return true;
}

static jboolean EnginePrepare(JNIEnv* env, jclass clzz, jobject assetsMgr/*AAssetManager*/,
	jstring filesRoot, jobjectArray plugins /*String[]*/)
{
	AAssetManager* mgr = AAssetManager_fromJava(env, assetsMgr);
	int count = env->GetArrayLength(plugins);
	vector<string> _plugins;
	for (int i = 0; i < count; i++ ) {
		jstring ck_plugin = (jstring)env->GetObjectArrayElement(plugins, i);
		string _plgin;
		GET_CPPSTR_FROM_JSTR(env, ck_plugin, _plgin);
		_plugins.push_back(_plgin);
	}
	string filesroot_str;
	GET_CPPSTR_FROM_JSTR(env, filesRoot, filesroot_str);

	try {
		Vm::android_init(mgr, filesroot_str.c_str(), _plugins);
		return true;
	}
	catch( const vedit::Exception& e ) {
		gHook.last_error = "engine prepare failed. cause:";
		gHook.last_error += e.what();
		return false;
	}
}

static jstring LastError(JNIEnv* env, jclass clazz)
{
	if (gHook.last_error.size()) {
		jstring jret = env->NewStringUTF(gHook.last_error.c_str());
		return jret;
	}
	else {
		return env->NewStringUTF("OK");
	}
}


static jboolean LoadJFileDir(JNIEnv* env, jclass clzz, jobject dirFile/*java/io/File*/)
{
	string dirPath,failinfo;
	dirPath = GetPathFromJFile(env, dirFile, failinfo);
	if (dirPath.size() == 0 ) {
		ostringstream oss;
		oss << "loadFileDir failed, cause:"<<failinfo;
		gHook.last_error = oss.str();
		return false;
	}

	try {
		Vm::load_script_dir(dirPath.c_str());
		gHook.last_error.clear();
		return true;
	}
	catch(const vedit::Exception& e)
	{
		ostringstream oss;
		oss << "loadFileDir failed, cause:"<<e.what();
		gHook.last_error = oss.str();
		return false;
	}
}

static jboolean LoadAssetDir(JNIEnv* env, jclass clazz, jobject assetmgr/*AssetManager*/, jstring assetDir)
{
	if (assetDir == NULL || assetmgr == NULL) {
		gHook.last_error = "empty arguments wen loadAssetDir";
		return false;
	}

	string dir;
	GET_CPPSTR_FROM_JSTR(env, assetDir, dir);

	if (dir.size() == 0) {
		gHook.last_error = "empty arguments wen loadAssetDir";
		return false;
	}

	AAssetManager* mgr = AAssetManager_fromJava(env, assetmgr);

	try {
		Vm::load_script_assets_dir(mgr, dir.c_str());
		return true;
	}
	catch(const vedit::Exception& e)
	{
		ostringstream oss;
		oss << "loadFileDir failed, cause:"<<e.what();
		gHook.last_error = oss.str();
		return false;
	}
}

static jboolean LoadFile(JNIEnv* env, jclass clazz, jobject jfile/*java/io/File*/)
{
	if (jfile == NULL) {
		gHook.last_error = "empty arguments when loadFile";
		return false;
	}

	string fail;
	json_t* obj = LoadJsonFromJFile(env, jfile, fail);
	if ( obj == 0) {
		gHook.last_error = "json parse failed when loadFile, cause:";
		gHook.last_error += fail;
		return false;
	}

	JsonWrap wrap(obj, 1);

	try {
		Vm::regist_script(obj);
		return true;
	}
	catch(const vedit::Exception& e) {
		gHook.last_error = "loadFile failed, cause:";
		gHook.last_error += e.what();
		return false;
	}
}

static jboolean LoadJson(JNIEnv* env, jclass clazz, jstring tmplt_json)
{
	if (tmplt_json == NULL) {
		gHook.last_error = "empty arguments when loadJson";
		return false;
	}

	string fail;
	json_t* obj = LoadJsonFromJString(env, tmplt_json, fail);
	if ( obj == 0) {
		gHook.last_error = "parse json failed when loadJson,cause:";
		gHook.last_error += fail;
		return false;
	}

	JsonWrap wrap(obj, 1);

	try {
		Vm::regist_script(obj);
		return true;
	}
	catch(const vedit::Exception& e) {
		gHook.last_error = "loadFile failed, cause:";
		gHook.last_error += e.what();
		return false;
	}
}

static jobjectArray ExpandScript(JNIEnv* env, jclass clazz, jstring tmplt_nm, jstring tmplt_arg_json)
{
	if (tmplt_nm == NULL) {
		gHook.last_error = "empty arguments when expand template";
		return NULL;
	}

	string tmplt_nm_str;
	GET_CPPSTR_FROM_JSTR(env, tmplt_nm, tmplt_nm_str);

	if (tmplt_nm_str.size() == 0) {
		gHook.last_error = "empty arguments when expand template";
		return NULL;
	}

	json_t* arg_js = NULL;
	if (tmplt_arg_json != NULL) {
		string fail;
		arg_js = LoadJsonFromJString(env, tmplt_arg_json, fail);
	}

	JsonWrap wrap(arg_js, 1);

	try {
		ScriptSerialized pair = Vm::call_script(tmplt_nm_str.c_str(), arg_js);
		jobjectArray jaret = env->NewObjectArray(2, gHook.java_string_clazz, NULL);
		env->SetObjectArrayElement(jaret,0, env->NewStringUTF(pair.first));
		char* str =  json_dumps(pair.second.h, 0);
		env->SetObjectArrayElement(jaret, 1, env->NewStringUTF(str));
		free(str);
		return jaret;
	}
	catch(const vedit::Exception& e) {
		gHook.last_error = "expand template failed ,cause: ";
		gHook.last_error += e.what();
		return NULL;
	}
}

static jobjectArray ExpandTypeScript(JNIEnv* env, jclass clazz, jstring tmplt_nm, jstring tmplt_type, jstring tmplt_arg_json)
{
	if ( tmplt_nm == NULL || tmplt_type == NULL) {
		gHook.last_error = "empty arguments when expand template";
		return NULL;
	}

	string tmplt_nm_str, tmplt_type_str;
	GET_CPPSTR_FROM_JSTR(env, tmplt_nm, tmplt_nm_str);
	GET_CPPSTR_FROM_JSTR(env, tmplt_type, tmplt_type_str);

	if ( tmplt_nm_str.size() == 0 || tmplt_type_str.size() == 0) {
		gHook.last_error = "empty arguments when expand template";
		return NULL;
	}

	vedit::ScriptType tp = vedit::AUDIO_RESOURCE_SCRIPT;
	for ( ; tp != vedit::INVALID_SCRIPT; tp = (vedit::ScriptType)((int)tp + 1) ) {
		if ( tmplt_type_str == string(Vm::proc_type_names[tp]) ) {
			break;
		}
	}

	if ( tp == vedit::INVALID_SCRIPT ) {
		gHook.last_error = "invalid template type when expand template";
		return NULL;
	}

	json_t* arg_js = NULL;
	if (tmplt_arg_json != NULL) {
		string fail;
		arg_js = LoadJsonFromJString(env, tmplt_arg_json, fail);
	}

	JsonWrap wrap(arg_js, 1);

	try {
		ScriptSerialized pair = Vm::call_script(tmplt_nm_str.c_str(), tp, arg_js);
		jobjectArray jaret = env->NewObjectArray(2, gHook.java_string_clazz, NULL);
		env->SetObjectArrayElement(jaret,0, env->NewStringUTF(pair.first));
		char* str =  json_dumps(pair.second.h, 0);
		env->SetObjectArrayElement(jaret, 1, env->NewStringUTF(str));
		free(str);
		return jaret;
	}
	catch(const vedit::Exception& e) {
		gHook.last_error = "expand template failed ,cause: ";
		gHook.last_error += e.what();
		return NULL;
	}
}

static JNINativeMethod gMethods[] = {
	{"initNative", "(Ljava/lang/String;Ljava/lang/String;I)Z", (void*)Init},
	{"enginePrepare", "(Landroid/content/res/AssetManager;Ljava/lang/String;[Ljava/lang/String;)Z", (void*)EnginePrepare},
	{"lastErrorNative", "()Ljava/lang/String;", (void*)LastError},
	{"loadFileDir", "(Ljava/io/File;)Z", (void*)LoadJFileDir},
	{"loadAssetDir", "(Landroid/content/res/AssetManager;Ljava/lang/String;)Z", (void*)LoadAssetDir},
	{"loadFile", "(Ljava/io/File;)Z", (void*)LoadFile},
	{"loadJson", "(Ljava/lang/String;)Z", (void*)LoadJson},
	{"expand", "(Ljava/lang/String;Ljava/lang/String;)[Ljava/lang/String;", (void*)ExpandScript},
	{"expandType", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)[Ljava/lang/String", (void*)ExpandTypeScript}
};

bool VEditVMJniLoad(JNIEnv* jnienv)
{
	FIND_JAVA_CLASS(jnienv, gHook.clazz, CLASSNAME, false);
	FIND_JAVA_CLASS(jnienv, gHook.java_io_file_clazz, "java/io/File", false);
	FIND_JAVA_CLASS(jnienv, gHook.java_string_clazz, "java/lang/String", false);
	FIND_JAVA_METHOD(jnienv, gHook.java_io_file_getPath_method, gHook.java_io_file_clazz,
		"getPath", "()Ljava/lang/String;",false);
	jnienv->RegisterNatives(gHook.clazz, gMethods,sizeof(gMethods)/sizeof(JNINativeMethod));
}

NMSP_END(vedit)

#endif
