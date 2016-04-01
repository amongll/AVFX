/*
 * VEditMltRuntime_andriod.cpp
 *
 *  Created on: 2016年3月29日
 *      Author: li.lei@youku.com
 */


#ifdef __ANDROID__
#include "framework/mlt.h"
#include "VEditMltRun.h"
#include <android/native_window_jni.h>

#define CLASSNAME "com/youku/cloud/vedit/MltRuntime"

NMSP_BEGIN(vedit)

static struct JNIJavaHook {
	jclass clazz;
	jmethodID get_native_method;
	jfieldID native_ptr_field;

	jclass java_io_file_clazz;
	jmethodID java_io_file_getPath_method;
} gHook;

struct MltRuntimeJNIWrap
{
	std::tr1::shared_ptr<MltRuntime> impl;
	string last_error;
};

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

static jstring RuntimeLastError( JNIEnv* env, jobject thiz )
{
	MltRuntimeJNIWrap* wrp = reinterpret_cast<MltRuntimeJNIWrap*>(
		env->GetLongField( thiz, gHook.native_ptr_field));
	if (wrp == NULL ) {
		jstring jret = env->NewStringUTF("native null");
		return jret;
	}

	if ( wrp->last_error.size() ) {
		jstring jret = env->NewStringUTF(wrp->last_error.c_str());
		return jret;
	}
	else {
		jstring jret = env->NewStringUTF("OK");
		return jret;
	}
}

static jlong CreateRuntime_fromPath( JNIEnv* env, jobject thiz, jobject file_path/*java/io/File*/ )
{
	MltRuntimeJNIWrap* wrp = reinterpret_cast<MltRuntimeJNIWrap*>(
		env->GetLongField( thiz, gHook.native_ptr_field));

	if (wrp == NULL) {
		wrp = new MltRuntimeJNIWrap();
		env->SetLongField(thiz, gHook.native_ptr_field, reinterpret_cast<jlong>(wrp));
	}
	else if (wrp->impl.get()) {
		wrp->impl.reset();
	}

	string err;
	json_t* serialize_js = LoadJsonFromJFile(env, file_path, err);
	if (serialize_js == NULL) {
		ostringstream oss;
		oss << "Create MltRuntime native impl failed when parse serialized json file, cause:"<<err;
		wrp->last_error = oss.str();
		return false;
	}

	try {
		MltRuntime* impl = new MltRuntime(serialize_js, 1);
		wrp->impl.reset(impl);
	}
	catch(const vedit::Exception& e)
	{
		ostringstream oss;
		oss << "Create MltRuntime native impl failed when create runtime instance, cause:"<<e.what();
		wrp->last_error = oss.str();
		return false;
	}

	wrp->last_error.clear();
	return true;
}

static jboolean CreateRuntime( JNIEnv* env, jobject thiz, jstring js_str )
{
	MltRuntimeJNIWrap* wrp = reinterpret_cast<MltRuntimeJNIWrap*>(
		env->GetLongField( thiz, gHook.native_ptr_field));

	if (wrp == NULL) {
		wrp = new MltRuntimeJNIWrap();
		env->SetLongField(thiz, gHook.native_ptr_field, reinterpret_cast<jlong>(wrp));
	}
	else if (wrp->impl.get()) {
		wrp->impl.reset();
	}

	string fail_info;
	json_t* jsobj = LoadJsonFromJString(env, js_str, fail_info);

	if ( jsobj == NULL ) {
		ostringstream oss;
		oss << "Create MltRuntime native impl failed when parse serialized json string, cause:"<<fail_info;
		wrp->last_error = oss.str();
		return false;
	}

	try {
		MltRuntime* impl = new MltRuntime(jsobj, 1);
		wrp->impl.reset(impl);
	}
	catch(const vedit::Exception& e)
	{
		ostringstream oss;
		oss << "Create MltRuntime native impl failed when create runtime instance, cause:"<<e.what();
		wrp->last_error = oss.str();
		return false;
	}

	wrp->last_error.clear();
	return true;
}

static jboolean AddRuntimeEntry( JNIEnv* env, jobject thiz, jstring entry_json, jstring json_path )
{
	MltRuntimeJNIWrap* wrp = reinterpret_cast<MltRuntimeJNIWrap*>(
		env->GetLongField( thiz, gHook.native_ptr_field));

	if (wrp == NULL) {
		wrp = new MltRuntimeJNIWrap();
		env->SetLongField(thiz, gHook.native_ptr_field, reinterpret_cast<jlong>(wrp));

		wrp->last_error = "native implement not inited yet when MltRuntime.addRuntimeEntry";
		return false;
	}
	else if ( wrp->impl.get() == NULL) {
		wrp->last_error = "native implement not inited yet when MltRuntime.addRuntimeEntry";
		return false;
	}

	string fail_info;
	json_t* added_entry = LoadJsonFromJString(env, entry_json, fail_info);
	if ( added_entry == NULL ) {
		ostringstream oss;
		oss << "Added runtime entry json parse failed when MltRuntime.addRuntimeEntry, cause:" << fail_info;
		wrp->last_error = oss.str();
		return false;
	}

	if (json_path == NULL) {
		ostringstream oss;
		oss << "MltRuntime.addRuntimeEntry failed, cause: json path empty";
		wrp->last_error = oss.str();
		return false;
	}

	string _json_path;
	GET_CPPSTR_FROM_JSTR(env, json_path, _json_path);

	try {
		JsonPath jspath_parse(_json_path.c_str());
		wrp->impl->add_runtime_entry(jspath_parse,added_entry, 1);
	}
	catch(const vedit::Exception& e)
	{
		ostringstream oss;
		oss << "Add runtime entry failed, cause:"<< e.what();
		wrp->last_error = oss.str();
		return false;
	}

	wrp->last_error.clear();
	return true;
}

static jboolean ReplaceRuntimeEntryUUID( JNIEnv* env, jobject thiz, jstring entry_json, jstring juuid )
{
	MltRuntimeJNIWrap* wrp = reinterpret_cast<MltRuntimeJNIWrap*>(
		env->GetLongField( thiz, gHook.native_ptr_field));

	if (wrp == NULL) {
		wrp = new MltRuntimeJNIWrap();
		env->SetLongField(thiz, gHook.native_ptr_field, reinterpret_cast<jlong>(wrp));

		wrp->last_error = "native implement not inited yet when MltRuntime.ReplaceRuntimeEntryUUID";
		return false;
	}
	else if ( wrp->impl.get() == NULL) {
		wrp->last_error = "native implement not inited yet when MltRuntime.ReplaceRuntimeEntryUUID";
		return false;
	}

	string fail_info;
	json_t* added_entry = LoadJsonFromJString(env, entry_json, fail_info);
	if ( added_entry == NULL ) {
		ostringstream oss;
		oss << "runtime entry json parse failed when MltRuntime.ReplaceRuntimeEntryUUID, cause:" << fail_info;
		wrp->last_error = oss.str();
		return false;
	}

	if (juuid == NULL) {
		ostringstream oss;
		oss << " MltRuntime.ReplaceRuntimeEntryUUID, cause: replace uuid emptry";
		wrp->last_error = oss.str();
		return false;
	}

	string uuid;
	GET_CPPSTR_FROM_JSTR(env, juuid, uuid);

	try {
		wrp->impl->replace_runtime_entry(uuid, added_entry, 1);
	}
	catch(const vedit::Exception& e)
	{
		ostringstream oss;
		oss << "replace runtime entry failed, cause:"<< e.what();
		wrp->last_error = oss.str();
		return false;
	}

	wrp->last_error.clear();
	return true;
}

static jboolean ReplaceRuntimeEntryJsonPath( JNIEnv* env, jobject thiz, jstring entry_json, jstring js_path )
{
	MltRuntimeJNIWrap* wrp = reinterpret_cast<MltRuntimeJNIWrap*>(
			env->GetLongField( thiz, gHook.native_ptr_field));

	if (wrp == NULL) {
		wrp = new MltRuntimeJNIWrap();
		env->SetLongField(thiz, gHook.native_ptr_field, reinterpret_cast<jlong>(wrp));

		wrp->last_error = "native implement not inited yet when MltRuntime.ReplaceRuntimeEntryJsonPath";
		return false;
	}
	else if ( wrp->impl.get() == NULL) {
		wrp->last_error = "native implement not inited yet when MltRuntime.ReplaceRuntimeEntryJsonPath";
		return false;
	}

	string fail_info;
	json_t* added_entry = LoadJsonFromJString(env, entry_json, fail_info);
	if ( added_entry == NULL ) {
		ostringstream oss;
		oss << "runtime entry json parse failed when MltRuntime.ReplaceRuntimeEntryJsonPath, cause:" << fail_info;
		wrp->last_error = oss.str();
		return false;
	}

	if (js_path == NULL) {
		ostringstream oss;
		oss << " MltRuntime.ReplaceRuntimeEntryJsonPath, cause: replace json path empty";
		wrp->last_error = oss.str();
		return false;
	}

	string json_path;
	GET_CPPSTR_FROM_JSTR(env, js_path, json_path);

	try {
		JsonPath jspath_obj(json_path.c_str());
		wrp->impl->replace_runtime_entry(jspath_obj, added_entry, 1);
	}
	catch(const vedit::Exception& e)
	{
		ostringstream oss;
		oss << "replace runtime entry failed, cause:"<< e.what();
		wrp->last_error = oss.str();
		return false;
	}

	wrp->last_error.clear();
	return true;
}

static jboolean DelRuntimeEntryUUID( JNIEnv* env, jobject thiz, jstring uuid )
{
	MltRuntimeJNIWrap* wrp = reinterpret_cast<MltRuntimeJNIWrap*>(
		env->GetLongField( thiz, gHook.native_ptr_field));

	if (wrp == NULL) {
		wrp = new MltRuntimeJNIWrap();
		env->SetLongField(thiz, gHook.native_ptr_field, reinterpret_cast<jlong>(wrp));

		wrp->last_error = "native implement not inited yet when MltRuntime.DelRuntimeEntryUUID";
		return false;
	}
	else if ( wrp->impl.get() == NULL) {
		wrp->last_error = "native implement not inited yet when MltRuntime.DelRuntimeEntryUUID";
		return false;
	}

	if (uuid == NULL) {
		ostringstream oss;
		oss << " MltRuntime.DelRuntimeEntryUUID, cause: deleted uuid empty";
		wrp->last_error = oss.str();
		return false;
	}

	string json_path;
	GET_CPPSTR_FROM_JSTR(env, uuid, json_path);

	try {
		wrp->impl->erase_runtime_entry(json_path);
	}
	catch(const vedit::Exception& e)
	{
		ostringstream oss;
		oss << "del runtime entry failed, cause:"<< e.what();
		wrp->last_error = oss.str();
		return false;
	}

	wrp->last_error.clear();
	return true;
}

static jboolean DelRuntimeEntryJsonPath( JNIEnv* env, jobject thiz, jstring js_path )
{
	MltRuntimeJNIWrap* wrp = reinterpret_cast<MltRuntimeJNIWrap*>(
		env->GetLongField( thiz, gHook.native_ptr_field));

	if (wrp == NULL) {
		wrp = new MltRuntimeJNIWrap();
		env->SetLongField(thiz, gHook.native_ptr_field, reinterpret_cast<jlong>(wrp));

		wrp->last_error = "native implement not inited yet when MltRuntime.DelRuntimeEntryJsonPath";
		return false;
	}
	else if ( wrp->impl.get() == NULL) {
		wrp->last_error = "native implement not inited yet when MltRuntime.DelRuntimeEntryJsonPath";
		return false;
	}

	if (js_path == NULL) {
		ostringstream oss;
		oss << " MltRuntime.DelRuntimeEntryJsonPath, cause: deleted json path empty";
		wrp->last_error = oss.str();
		return false;
	}

	string json_path;
	GET_CPPSTR_FROM_JSTR(env, js_path, json_path);

	try {
		JsonPath jsobj(json_path.c_str());
		wrp->impl->erase_runtime_entry(jsobj);
	}
	catch(const vedit::Exception& e)
	{
		ostringstream oss;
		oss << "del runtime entry failed, cause:"<< e.what();
		wrp->last_error = oss.str();
		return false;
	}

	wrp->last_error.clear();
	return true;
}

static void DestroyRuntime( JNIEnv* env, jobject thiz)
{
	MltRuntimeJNIWrap* wrp = reinterpret_cast<MltRuntimeJNIWrap*>(
		env->GetLongField( thiz, gHook.native_ptr_field));

	if (wrp) {
		env->SetLongField( thiz, gHook.native_ptr_field, 0);
		delete wrp;
	}
}

static jboolean RuntimePrepare( JNIEnv* env, jobject thiz )
{
	MltRuntimeJNIWrap* wrp = reinterpret_cast<MltRuntimeJNIWrap*>(
		env->GetLongField( thiz, gHook.native_ptr_field));

	if (wrp == NULL) {
		wrp = new MltRuntimeJNIWrap();
		env->SetLongField(thiz, gHook.native_ptr_field, reinterpret_cast<jlong>(wrp));

		wrp->last_error = "native implement not inited yet when MltRuntime.prepareNative";
		return false;
	}
	else if ( wrp->impl.get() == NULL) {
		wrp->last_error = "native implement not inited yet when MltRuntime.prepareNative";
		return false;
	}

	try {
		wrp->impl->init();
	}
	catch(const vedit::Exception& e ) {
		ostringstream oss;
		oss << "prepare MltRuntime native failed,cause"<<e.what();
		wrp->last_error = oss.str();
		return false;
	}

	wrp->last_error.clear();
	return true;
}

static jboolean RuntimeRunning( JNIEnv* env, jobject thiz )
{
	MltRuntimeJNIWrap* wrp = reinterpret_cast<MltRuntimeJNIWrap*>(
		env->GetLongField( thiz, gHook.native_ptr_field));
	if (wrp == NULL) {
		wrp = new MltRuntimeJNIWrap();
		env->SetLongField(thiz, gHook.native_ptr_field, reinterpret_cast<jlong>(wrp));

		wrp->last_error = "native implement not inited yet when MltRuntime.runningNative";
		return false;
	}
	else if ( wrp->impl.get() == NULL) {
		wrp->last_error = "native implement not inited yet when MltRuntime.runningNative";
		return false;
	}

	return wrp->impl->running();
}

static jboolean RuntimeSeek( JNIEnv* env, jobject thiz, jlong pos)
{
	MltRuntimeJNIWrap* wrp = reinterpret_cast<MltRuntimeJNIWrap*>(
		env->GetLongField( thiz, gHook.native_ptr_field));

	if (wrp == NULL) {
		wrp = new MltRuntimeJNIWrap();
		env->SetLongField(thiz, gHook.native_ptr_field, reinterpret_cast<jlong>(wrp));

		wrp->last_error = "native implement not inited yet when MltRuntime.seekNative";
		return false;
	}
	else if ( wrp->impl.get() == NULL) {
		wrp->last_error = "native implement not inited yet when MltRuntime.seekNative";
		return false;
	}

	try {
		wrp->impl->seek(pos);
	}
	catch(const vedit::Exception& e ) {
		ostringstream oss;
		oss << "seek MltRuntime native failed,cause"<<e.what();
		wrp->last_error = oss.str();
		return false;
	}

	wrp->last_error.clear();
	return true;
}

static jboolean RuntimeSetSpeed( JNIEnv* env, jobject thiz, jdouble speed)
{
	MltRuntimeJNIWrap* wrp = reinterpret_cast<MltRuntimeJNIWrap*>(
		env->GetLongField( thiz, gHook.native_ptr_field));
	if (wrp == NULL) {
		wrp = new MltRuntimeJNIWrap();
		env->SetLongField(thiz, gHook.native_ptr_field, reinterpret_cast<jlong>(wrp));

		wrp->last_error = "native implement not inited yet when MltRuntime.setSpeedNative";
		return false;
	}
	else if ( wrp->impl.get() == NULL) {
		wrp->last_error = "native implement not inited yet when MltRuntime.setSpeedNative";
		return false;
	}

	try {
		wrp->impl->set_speed(speed);
	}
	catch(const vedit::Exception& e ) {
		ostringstream oss;
		oss << "setSpeedNative MltRuntime native failed,cause"<<e.what();
		wrp->last_error = oss.str();
		return false;
	}

	wrp->last_error.clear();
	return true;
}

static void RuntimeStop( JNIEnv* env, jobject thiz )
{
	MltRuntimeJNIWrap* wrp = reinterpret_cast<MltRuntimeJNIWrap*>(
		env->GetLongField( thiz, gHook.native_ptr_field));

	if (wrp == NULL) {
		wrp = new MltRuntimeJNIWrap();
		env->SetLongField(thiz, gHook.native_ptr_field, reinterpret_cast<jlong>(wrp));

		wrp->last_error = "native implement not inited yet when MltRuntime.stopNative";
	}
	else if ( wrp->impl.get() == NULL) {
		wrp->last_error = "native implement not inited yet when MltRuntime.stopNative";
	}

	try {
		wrp->impl->stop();
	}
	catch(const vedit::Exception& e)
	{
		ostringstream oss;
		oss << "stop MltRuntime native failed,cause"<<e.what();
		wrp->last_error = oss.str();
	}
	return;
}

static jboolean RuntimeRun( JNIEnv* env, jobject thiz, jobject surface )
{
	MltRuntimeJNIWrap* wrp = reinterpret_cast<MltRuntimeJNIWrap*>(
		env->GetLongField( thiz, gHook.native_ptr_field));

	if (wrp == NULL) {
		wrp = new MltRuntimeJNIWrap();
		env->SetLongField(thiz, gHook.native_ptr_field, reinterpret_cast<jlong>(wrp));

		wrp->last_error = "native implement not inited yet when MltRuntime.runNative";
		return false;
	}
	else if ( wrp->impl.get() == NULL) {
		wrp->last_error = "native implement not inited yet when MltRuntime.runNative";
		return false;
	}

	if (surface == NULL) {
		wrp->last_error = "surface null for MltRuntime run in surface.";
		return false;
	}

	try {
		ANativeWindow* nw = ANativeWindow_fromSurface(env, surface);
		wrp->impl->run(nw);
	}
	catch(const vedit::Exception& e ) {
		ostringstream oss;
		oss << "run MltRuntime native failed,cause"<<e.what();
		wrp->last_error = oss.str();
		return false;
	}

	wrp->last_error.clear();
	return true;
}

static jboolean RuntimeFileGen( JNIEnv* env, jobject thiz, jobject outPath /*java/io/File*/ )
{
	MltRuntimeJNIWrap* wrp = reinterpret_cast<MltRuntimeJNIWrap*>(
		env->GetLongField( thiz, gHook.native_ptr_field));

	if (wrp == NULL) {
		wrp = new MltRuntimeJNIWrap();
		env->SetLongField(thiz, gHook.native_ptr_field, reinterpret_cast<jlong>(wrp));

		wrp->last_error = "native implement not inited yet when MltRuntime.outputNative";
		return false;
	}
	else if ( wrp->impl.get() == NULL) {
		wrp->last_error = "native implement not inited yet when MltRuntime.outputNative";
		return false;
	}

	if (outPath == NULL) {
		wrp->last_error = "File null for MltRuntime output file.";
		return false;
	}

	try {
		string fail;
		string filepath = GetPathFromJFile(env, outPath, fail);
		wrp->impl->run(filepath);
	}
	catch(const vedit::Exception& e ) {
		ostringstream oss;
		oss << "gen file MltRuntime native failed,cause"<<e.what();
		wrp->last_error = oss.str();
		return false;
	}

	wrp->last_error.clear();
	return true;
}

static jlong RuntimeGetFrameLength( JNIEnv* env, jobject thiz )
{
	MltRuntimeJNIWrap* wrp = reinterpret_cast<MltRuntimeJNIWrap*>(
		env->GetLongField( thiz, gHook.native_ptr_field));

	if (wrp == NULL) {
		wrp = new MltRuntimeJNIWrap();
		env->SetLongField(thiz, gHook.native_ptr_field, reinterpret_cast<jlong>(wrp));

		wrp->last_error = "native implement not inited yet when MltRuntime.getFrameLengthNative";
		return false;
	}
	else if ( wrp->impl.get() == NULL) {
		wrp->last_error = "native implement not inited yet when MltRuntime.getFrameLengthNative";
		return false;
	}

	try {
		return wrp->impl->get_frame_length();
	}
	catch(const vedit::Exception& e)
	{
		ostringstream oss;
		oss << "get MltRuntime frame length failed,cause:"<<e.what();
		wrp->last_error = oss.str();
		return -1;
	}
}

static jlong RuntimeGetFramePosition( JNIEnv* env, jobject thiz )
{
	MltRuntimeJNIWrap* wrp = reinterpret_cast<MltRuntimeJNIWrap*>(
		env->GetLongField( thiz, gHook.native_ptr_field));

	if (wrp == NULL) {
		wrp = new MltRuntimeJNIWrap();
		env->SetLongField(thiz, gHook.native_ptr_field, reinterpret_cast<jlong>(wrp));

		wrp->last_error = "native implement not inited yet when MltRuntime.getFramePositionNative";
		return false;
	}
	else if ( wrp->impl.get() == NULL) {
		wrp->last_error = "native implement not inited yet when MltRuntime.getFramePositionNative";
		return false;
	}

	try {
		return wrp->impl->get_frame_position();
	}
	catch(const vedit::Exception& e)
	{
		ostringstream oss;
		oss << "get MltRuntime frame position failed,cause:"<<e.what();
		wrp->last_error = oss.str();
		return -1;
	}
}

static jstring GetRuntimeEntryJsonPath( JNIEnv* env, jobject thiz, jstring juuid )
{
	MltRuntimeJNIWrap* wrp = reinterpret_cast<MltRuntimeJNIWrap*>(
			env->GetLongField( thiz, gHook.native_ptr_field));

	if (wrp == NULL) {
		wrp = new MltRuntimeJNIWrap();
		env->SetLongField(thiz, gHook.native_ptr_field, reinterpret_cast<jlong>(wrp));

		wrp->last_error = "native implement not inited yet when MltRuntime.DelRuntimeEntryUUID";
		return false;
	}
	else if ( wrp->impl.get() == NULL) {
		wrp->last_error = "native implement not inited yet when MltRuntime.DelRuntimeEntryUUID";
		return false;
	}

	if (juuid == NULL) {
		ostringstream oss;
		oss << " MltRuntime.getRuntimeEntryJsonPath, cause: uuid empty";
		wrp->last_error = oss.str();
		return false;
	}

	string json_path;
	GET_CPPSTR_FROM_JSTR(env, juuid, json_path);

	const JsonPath* ret = wrp->impl->get_runtime_entry_path(json_path);
	if ( ret == NULL) return NULL;

	jstring jret = env->NewStringUTF(ret->str().c_str());
	return jret;
}

static JNINativeMethod gMethods[] = {
	{"createNative", "(Ljava/lang/String;)Z", (void*)CreateRuntime},
	{"createNative", "(Ljava/io/File;)Z", (void*)CreateRuntime_fromPath},
	{"destroyNative", "()V", (void*)DestroyRuntime},
	{"addRuntimeEntry","(Ljava/lang/String;Ljava/lang/String;)Z",(void*)AddRuntimeEntry},
	{"replaceRuntimeEntryUUID","(Ljava/lang/String;Ljava/lang/String;)Z",(void*)ReplaceRuntimeEntryUUID},
	{"replaceRuntimeEntryJsonPath","(Ljava/lang/String;Ljava/lang/String;)Z",(void*)ReplaceRuntimeEntryJsonPath},
	{"delRuntimeEntryUUID","(Ljava/lang/String;)Z",(void*)DelRuntimeEntryUUID},
	{"delRuntimeEntryJsonPath","(Ljava/lang/String;)Z",(void*)DelRuntimeEntryJsonPath},
	{"prepareNative","()Z",(void*)RuntimePrepare},
	{"runNative","(Landroid/view/Surface;)Z",(void*)RuntimeRun},
	{"outputNative","(Ljava/io/File;)Z",(void*)RuntimeFileGen},
	{"seekNative","(J)Z",(void*)RuntimeSeek},
	{"setSpeedNative","(D)Z",(void*)RuntimeSetSpeed},
	{"getFramePositionNative","()J",(void*)RuntimeGetFramePosition},
	{"getFrameLengthNative","()J",(void*)RuntimeGetFrameLength},
	{"runningNative","()Z",(void*)RuntimeRunning},
	{"stopNative","()V",(void*)RuntimeStop},
	{"lastErrorNative", "()Ljava/lang/String;", (void*)RuntimeLastError},
	{"getRuntimeEntryJsonPath", "()Ljava/lang/String;", (void*)GetRuntimeEntryJsonPath}
};

bool VEditMltRuntimeJniLoad(JNIEnv* jnienv)
{
	FIND_JAVA_CLASS(jnienv, gHook.clazz, CLASSNAME, false);
	FIND_JAVA_METHOD(jnienv, gHook.get_native_method, gHook.clazz, "getNative", "()J",false);
	FIND_JAVA_FIELD(jnienv, gHook.native_ptr_field, gHook.clazz, "native_obj", "J", false);

	FIND_JAVA_CLASS(jnienv, gHook.java_io_file_clazz, "java/io/File", false);
	FIND_JAVA_METHOD(jnienv, gHook.java_io_file_getPath_method, gHook.java_io_file_clazz, "getPath", "()Ljava/lang/String;",false);

	jnienv->RegisterNatives(gHook.clazz, gMethods,sizeof(gMethods)/sizeof(JNINativeMethod));
}

NMSP_END(vedit)

#endif

