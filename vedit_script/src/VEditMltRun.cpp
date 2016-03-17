/*
 * VEditMltRun.cpp
 *
 *  Created on: 2016-2-26
 *      Author: li.lei@youku.com
 */


#include "VEditMltRun.h"


NMSP_BEGIN(vedit)

JsonPathComponent::JsonPathComponent(const char* desc):
	type(PathInvalid),
	arr_idx(0)
{
	if (!desc || !strlen(desc) ) {
		type = PathObject;
	}
	else {
		string desc_obj (desc);

		size_t pos = desc_obj.rfind(':');
		if ( pos == string::npos || pos == 0 || pos == strlen(desc) - 1 ) {
			type = PathObject;
			name = desc;
		}
		else {
			string idx = desc_obj.substr(pos+1);
			char* ep = NULL;
			long i = strtol(idx.c_str(), &ep, 10);
			if ( *ep ) {
				type = PathObject;
				name = desc;
			}
			else {
				type = PathArray;
				name = desc_obj.substr(pos);
				arr_idx = i;
			}
		}
	}
}

JsonPath::JsonPath(const char* desc)
{
	if (desc && strlen(desc) ) {
		char* cp = strdup(desc);
		char* pp = NULL;
		char* tk = strtok_r(cp, "/", &pp);
		for ( ; tk; tk = strtok_r(NULL, "/", &pp) ) {
			path.push_back(tk);
		}
		free(cp);
	}
}

JsonPath::JsonPath(const JsonPath& r):
	path(r.path)
{
}

JsonPath& JsonPath::operator =(const JsonPath& r)
{
	path = r.path;
	return *this;
}

JsonPath JsonPath::operator + (const JsonPath& r)
{
	JsonPath ret(*this);
	ret.path.insert(ret.path.end(), r.path.begin(), r.path.end());
	return ret;
}

hash_map<string, pair<MltLoader*,MltLoader::LoadMltMemFp> > MltLoader::loader_regists;

std::string JsonPath::str() const
{
	ostringstream oss;
	PathCompCIter it;
	for ( it = path.begin(); it != path.end(); it++ ) {
		oss << "/";
		if ( it->type == JsonPathComponent::PathObject ) {
			oss << it->name;
		}
		else if ( it->type == JsonPathComponent::PathArray ) {
			oss << it->name << ":" << it->arr_idx;
		}
	}

	return oss.str();
}

pthread_mutex_t MltLoader::mlt_register_lock = PTHREAD_MUTEX_INITIALIZER;
hash_map<string,MltSvcWrap> MltLoader::mlt_register;

void MltLoader::push_mlt_registry(mlt_service obj, const char* uuid)
{
	if (!obj || !uuid || !strlen(uuid)) return;

	Lock lk(&mlt_register_lock);
	mlt_register[string(uuid)] = MltSvcWrap(obj, 1);
}

mlt_service MltLoader::pop_mlt_registry(const char* uuid)
{
	if (!uuid || !strlen(uuid) ) return NULL;

	mlt_service ret = NULL;
	{
		Lock lk(&mlt_register_lock);
		hash_map<string, MltSvcWrap>::iterator it = mlt_register.find(string(uuid));
		if ( it != mlt_register.end() ) {
			ret = it->second.obj;
			mlt_properties_inc_ref(mlt_service_properties(ret));
			mlt_register.erase(it);
		}
	}
	return ret;
}

mlt_service MltLoader::load_mlt(JsonWrap arg) throw (Exception)
{
	json_t* js = arg.h;
	json_t* je = json_object_get(js, "proctype");
	if ( !je || !json_is_string(je) || !strlen(json_string_value(je)) ) {
		throw_error_v(ErrorImplError, "proctype unknown");
	}
	const char* proctype_s = json_string_value(je);

	hash_map<string, std::pair<MltLoader*, LoadMltMemFp> >::iterator it =
			loader_regists.find(string(proctype_s));

	if ( it == loader_regists.end() ) {
		throw_error_v(ErrorImplError, "proctype %s unknown", proctype_s);
	}

	return ((it->second.first)->*(it->second.second))(arg);
}

void MltRuntime::parse_struct(json_t* v, const JsonPath& curPath,
		hash_map<string, JsonPath>& uuid_paths, int erase) throw(Exception)
{
	json_t* je = json_object_get(v, "uuid");
	if ( je && json_is_string(je) && strlen(json_string_value(je)) ) {
		string uuid = json_string_value(je);
		if (!erase) {
			if ( uuid_paths.find(uuid) != uuid_paths.end()) {
				throw_error_v(ErrorRuntimeJsonUUIDDup, "uuid dup:%s", uuid.c_str());
			}
			uuid_paths[uuid] = curPath;
		}
		else {
			uuid_paths.erase(string(uuid));
		}
	}

	void* it =  json_object_iter(v);
	while(it) {
		const char* k = json_object_iter_key(it);
		je = json_object_iter_value(it);
		it = json_object_iter_next(v, it);

		if ( json_is_object(je) && json_object_size(je) > 0 ) {
			JsonPath subPath(curPath);
			subPath.push_back(k);
			parse_struct(je, subPath, uuid_paths, erase);
		}
		else if (json_is_array(je) && json_array_size(je) > 0) {
			int sz = json_array_size(je);
			for ( int i=0; i<sz; i++ ) {
				json_t* ae = json_array_get(je, i);
				if ( json_is_object(ae) && json_object_size(ae) > 0 ) {
					JsonPath subPath(curPath);
					subPath.push_back(k, i);
					parse_struct(ae, subPath, uuid_paths, erase);
				}
			}
		}
	}
}

MltRuntime::MltRuntime(json_t* script_serialed, int give) throw(Exception):
	json_version(0),
	producer_version(0),
	json_serialize(NULL),
	producer(NULL),
	consumer(NULL),
	status(StatusCreated)
{
	if ( !script_serialed || !json_is_object(script_serialed)
			|| !json_object_size(script_serialed)) {
		throw_error_v(ErrorImplError,"Init MltRuntime with empty json");
	}
	if (give)
		json_serialize = script_serialed;
	else
		json_serialize = json_incref(script_serialed);

	parse_struct(json_serialize, JsonPath(), uuid_pathmap);
	json_version++;
	pthread_mutex_init(&run_lock,NULL);
}

MltRuntime::~MltRuntime()
{
	try {
		stop();
	}
	catch(const Exception& e) {}

	if (producer)mlt_producer_close(producer);
	if (consumer)mlt_consumer_close(consumer);
	if (json_serialize) json_decref(json_serialize);
}

void MltRuntime::erase_runtime_entry(const string& uuid) throw(Exception)
{
	PathIter it = uuid_pathmap.find(uuid);
	if ( it == uuid_pathmap.end() ) {
		throw_error_v(ErrorRuntimeJsonUUIDNotFound,"%s", uuid.c_str());
	}

	erase_runtime_entry(it->second);
}

void MltRuntime::erase_runtime_entry(const JsonPath& path) throw(Exception)
{
	JsonPath::PathCompCIter it = path.path.begin();
	json_t* curObj = json_serialize;
	if ( path.path.size() == 0 ) {
		throw_error_v(ErrorRuntimeUuidPathInvalid, "root path not allowd for erase");
	}

	for (; it != path.path.end(); ) {
		json_t* se = json_object_get(curObj, it->name.c_str());
		if (!se) {
			throw_error(ErrorRuntimeUuidPathInvalid);
		}
		if ( it->type == JsonPathComponent::PathArray) {
			if ( !json_is_array(se) ) {
				throw_error(ErrorRuntimeUuidPathInvalid);
			}

			int asz = json_array_size(se);
			int idx = it->arr_idx;
			if (idx < 0) idx = asz + idx;
			if ( idx < 0 || idx >= asz ) {
				if (it->arr_idx >= asz) {
					throw_error(ErrorRuntimeUuidPathInvalid);
				}
			}

			json_t* ae = json_array_get(se, idx);
			if (!json_is_object(ae)) {
				throw_error(ErrorRuntimeUuidPathInvalid);
			}
			curObj = ae;
			it++;
			if ( it == path.path.end()) {
				JsonPath dummy;
				parse_struct(ae,dummy,uuid_pathmap, 1);
				json_array_remove(se, idx);
				break;
			}
			else
				continue;
		}
		else if ( it->type == JsonPathComponent::PathObject) {
			if ( !json_is_object(se)) {
				throw_error(ErrorRuntimeUuidPathInvalid);
			}

			it++;
			if ( it == path.path.end()) {
				JsonPath dummy;
				parse_struct(se, dummy, uuid_pathmap, 1);
				json_object_del(curObj, it->name.c_str());
				break;
			}
			else {
				curObj = se;
				continue;
			}
		}
	}

	json_version++;
	return;
}


void MltRuntime::add_runtime_entry(const JsonPath& path,
		json_t* script_serialed, int give) throw(Exception)
{
	if ( path.path.size() == 0) {
		throw_error_v(ErrorRuntimeUuidPathInvalid, "root path not allowed for add entry");
	}

	JsonPath parentPath(path);
	parentPath.path.pop_back();

	json_t* parent_je = json_serialize;
	JsonPath::PathCompIter it = parentPath.path.begin();

	for (; it != parentPath.path.end(); it++)
	{
		json_t* je = json_object_get(parent_je, it->name.c_str());
		if (!je) {
			throw_error_v(ErrorRuntimeUuidPathInvalid, "parent path invalid for add entry");
		}
		if ( it->type == JsonPathComponent::PathObject ) {
			if ( !json_is_object(je) ) {
				throw_error_v(ErrorRuntimeUuidPathInvalid, "parent path invalid for add entry");
			}
			parent_je = je;
		}
		else if (it->type == JsonPathComponent::PathArray ) {
			if (!json_is_array(je)) {
				throw_error_v(ErrorRuntimeUuidPathInvalid, "parent path invalid for add entry");
			}
			int sz = json_array_size(je);
			if (sz == 0) {
				throw_error_v(ErrorRuntimeUuidPathInvalid, "parent path invalid for add entry");
			}

			int idx = it->arr_idx;
			if (idx < 0) idx = sz + idx;
			if (idx < 0 || idx >= sz ) {
				throw_error_v(ErrorRuntimeUuidPathInvalid, "parent path invalid for add entry");
			}

			parent_je = json_array_get(je, idx);
		}
	}

	if (!json_is_object(parent_je)) {
		throw_error_v(ErrorRuntimeUuidPathInvalid, "parent path invalid for add entry");
	}

	const JsonPathComponent& lastPath = *(path.path.rbegin());
	if (lastPath.type == JsonPathComponent::PathArray) {
		json_t* cur = json_object_get(parent_je, lastPath.name.c_str());
		if ( !cur ) {
			cur = json_array();
			json_object_set_new(parent_je, lastPath.name.c_str(), cur);
		}

		if (!json_is_array(cur)) {
			throw_error_v(ErrorRuntimeUuidPathInvalid, " path invalid for add entry");
		}
		else {
			int idx = lastPath.arr_idx;
			int sz = json_array_size(cur);

			if (idx < 0) {
				idx = sz + idx + 1;
			}

			if (idx > sz) idx = sz;
			if (idx < 0) idx = 0;

			if ( idx == sz ) {
				JsonPath curPath(parentPath);
				curPath.push_back(lastPath.name.c_str(), idx);
				parse_struct(script_serialed, curPath, uuid_pathmap);
				if (give)
					json_array_append_new(cur, script_serialed);
				else
					json_array_append(cur, script_serialed);
			}
			else {
				JsonPath curPath(parentPath);
				curPath.push_back(lastPath.name.c_str(), idx);
				parse_struct(script_serialed, curPath, uuid_pathmap);
				if (give)
					json_array_insert_new(cur, idx, script_serialed);
				else
					json_array_insert(cur, idx, script_serialed);
			}
		}
	}
	else if (lastPath.type == JsonPathComponent::PathObject) {
		json_t* cur = json_object_get(parent_je, lastPath.name.c_str());
		if (cur) {
			throw_error_v(ErrorRuntimeUuidPathInvalid, " path invalid for add entry");
		}

		JsonPath curPath(parentPath);
		curPath.push_back(lastPath.name.c_str());
		parse_struct(script_serialed, curPath, uuid_pathmap);
		if (give) {
			json_object_set_new(parent_je, lastPath.name.c_str(), script_serialed);
		}
		else {
			json_object_set(parent_je, lastPath.name.c_str(), script_serialed);
		}
	}

	json_version++;
	return;
}

void MltRuntime::replace_runtime_entry(const string& src_uuid,
	json_t* script_serialed, int give) throw(Exception)
{
	PathIter it = uuid_pathmap.find(src_uuid);
	if ( it == uuid_pathmap.end()) {
		throw_error_v(ErrorRuntimeJsonUUIDNotFound,"%s", src_uuid.c_str());
	}
	JsonPath pcp(it->second);
	erase_runtime_entry(src_uuid);
	add_runtime_entry(pcp, script_serialed, give);
}


void MltRuntime::replace_runtime_entry(const JsonPath& path,
		json_t* script_seriled, int giv) throw (Exception)
{
	erase_runtime_entry(path);
	add_runtime_entry(path, script_seriled, giv);
}

const JsonPath* MltRuntime::get_runtime_entry_path(const string& uuid)
{
	PathCIter it = uuid_pathmap.find(uuid);
	return it == uuid_pathmap.end()? NULL : &it->second;
}

json_t* MltRuntime::run() throw (Exception)
{
	stop();
	Lock lk(&run_lock);
	if ( json_serialize == NULL)
		throw_error_v(ErrorImplError, "running with empty json_serailzed");

	json_t* uuid_je = json_object_get(json_serialize, "uuid");
	assert(uuid_je && json_is_string(uuid_je) && strlen(json_string_value(uuid_je)));

//	mlt_producer tmp_producer = MLT_PRODUCER( MltLoader::pop_mlt_registry(json_string_value(uuid_je)));
//	if (tmp_producer == NULL) {
	mlt_producer	tmp_producer = (mlt_producer) MltLoader::load_mlt(JsonWrap(json_serialize));
//	}

	producer_version = json_version;

	if (producer)mlt_producer_close(producer);
	if (consumer)mlt_consumer_close(consumer);

	producer = tmp_producer;
	mlt_profile profile = mlt_profile_init(NULL);
	consumer = mlt_factory_consumer(profile, "sdl", NULL); //todo

	//mlt_producer_optimise(producer);
	mlt_consumer_connect(consumer, mlt_producer_service(producer));
	mlt_consumer_start(consumer);
	status = StatusRunning;
	return json_incref(json_serialize);
}

void MltRuntime::seek(int framePos) throw (Exception)
{
	throw_error(ErrorFeatureNotImpl);
}

void MltRuntime::stop_ulk() throw (Exception)
{
	if (status == StatusRunning) {
		mlt_consumer_purge(consumer);
		mlt_consumer_stop(consumer);

		while ( mlt_consumer_is_stopped(consumer) == 0) {
			struct timespec req = {1,0};
			nanosleep(&req,NULL);
		}
		status = StatusStopped;

		mlt_consumer_close(consumer);
		consumer = NULL;
		mlt_producer_close(producer);
		producer = NULL;
	}
}
void MltRuntime::stop() throw (Exception)
{
	Lock lk(&run_lock);
	stop_ulk();
}

uint32_t MltRuntime::get_frame_length() throw (Exception)
{

}

uint32_t MltRuntime::get_frame_position() throw (Exception)
{

}

int MltRuntime::get_runtime_entry_property_int(const string& uuid,
		const string& procname)
{

}

int MltRuntime::get_runtime_entry_property_double(const string& uuid,
		const string& procname)
{

}

string MltRuntime::get_runtime_entry_property_string(const string& uuid,
		const string& procname)
{

}


NMSP_END(vedit)

