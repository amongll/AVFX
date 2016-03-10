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
	array_insert_idx(0)
{
	if (!desc || !strlen(desc) ) {
		type = PathObject;
	}
	else {
		size_t pos = string(desc).rfind(':');
		if ( pos == string::npos ) {
			type = PathObject;
			name = desc;
		}
		else {
			string idx = string(desc).substr(pos+1);
			if (idx.size() > 0) {
				char* ep = NULL;
				long i = strtol(idx.c_str(), &ep, 10);
				if ( *ep ) {
					type = PathObject;
					name = desc;
				}
				else {
					type = PathArray;
					name = string(desc).substr(pos);
					array_insert_idx = i;
				}
			}
			else {
				type = PathObject;
				name = desc;
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

JsonPath JsonPath::operator +(const JsonPath& l, const JsonPath& r)
{
	JsonPath ret(l);
	ret.path.insert(path.end(), r.path.begin(), r.path.end());
	return ret;
}

void MltRuntime::parse_struct(json_t* v, const JsonPath& curPath,
		hash_map<string, JsonPath>& uuid_paths) throw(Exception)
{
	json_t* je = json_object_get(v, "uuid");
	if ( je && json_is_string(je) && strlen(json_string_value(je)) ) {
		string uuid = json_string_value(je);
		if ( uuid_paths.find(uuid) != uuid_paths.end()) {
			throw Exception(ErrorRuntimeJsonUUIDDup, "uuid dup:%s", uuid.c_str());
		}

		uuid_paths[uuid] = curPath;
	}

	void* it =  json_object_iter(v);
	while(it) {
		const char* k = json_object_iter_key(it);
		je = json_object_iter_value(it);
		it = json_object_iter_next(v, it);

		if ( json_is_object(je) && json_object_size(je) > 0 ) {
			JsonPath subPath(curPath);
			subPath.push_back(k);
			parse_struct(je, subPath, uuid_paths);
		}
		else if (json_is_array(je) && json_array_size(je) > 0) {
			int sz = json_array_size(je);
			for ( int i=0; i<sz; i++ ) {
				json_t* ae = json_array_get(je, i);
				if ( json_is_object(ae) && json_object_size(ae) > 0 ) {
					JsonPath subPath(curPath);
					subPath.push_back(k, i);
					parse_struct(je, subPath, uuid_paths);
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
		throw Exception(ErrorImplError,"Init MltRuntime with empty json");
	}
	if (give)
		json_serialize = script_serialed;
	else
		json_serialize = json_incref(script_serialed);

	parse_struct(json_serialize, JsonPath(), uuid_pathmap);
}

MltRuntime::~MltRuntime()
{
	try {
		if ( is_stopped() ) {
			stop();
		}
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
		throw Exception(ErrorRuntimeJsonUUIDNotFound,"%s", uuid);
	}

	erase_runtime_entry(it->second);
}

void MltRuntime::erase_runtime_entry(const JsonPath& path) throw(Exception)
{
	JsonPath::PathCompCIter it = path.path.begin();
	json_t* curObj = json_serialize;
	if ( path.path.size() == 0 ) {
		throw Exception(ErrorRuntimeUuidPathInvalid, "root path not allowd for erase");
	}

	for (; it != path.path.end(); ) {
		json_t* se = json_object_get(curObj, it->name.c_str());
		if (!se) {
			throw Exception(ErrorRuntimeUuidPathInvalid);
		}
		if ( it->type == JsonPathComponent::PathArray) {
			if ( !json_is_array(se) ) {
				throw Exception(ErrorRuntimeUuidPathInvalid);
			}

			int asz = json_array_size(se);
			int idx = it->array_insert_idx;
			if (idx < 0) idx = asz + idx;
			if ( idx < 0 || idx >= asz ) {
				if (it->array_insert_idx >= asz) {
					throw Exception(ErrorRuntimeUuidPathInvalid);
				}
			}

			json_t* ae = json_array_get(se, idx);
			if (!json_is_object(ae)) {
				throw Exception(ErrorRuntimeUuidPathInvalid);
			}
			curObj = ae;
			it++;
			if ( it == path.path.end()) {
				json_array_remove(se, idx);
				break;
			}
			else
				continue;
		}
		else if ( it->type == JsonPathComponent::PathObject) {
			if ( !json_is_object(se)) {
				throw Exception(ErrorRuntimeUuidPathInvalid);
			}

			it++;
			if ( it == path.path.end()) {
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

void MltRuntime::erase_runtime_entry(const char* path) throw (Exception)
{
	JsonPath obj(path);
	erase_runtime_entry(obj);
}

void MltRuntime::add_runtime_entry(const JsonPath& path,
		json_t* script_serialed, int give) throw(Exception)
{
	if ( path.path.size() == 0) {
		throw Exception(ErrorRuntimeUuidPathInvalid, "root path not allowed for add entry");
	}

	JsonPath parentPath(path);
	parentPath.path.resize(parentPath.path.size() - 1);

	json_t* parent_je = json_serialize;
	JsonPath::PathCompIter it = parentPath.path.begin();

	for (; it != parentPath.path.end(); it++)
	{
		json_t* je = json_object_get(parent_je, it->name.c_str());
		if (!je) {
			throw Exception(ErrorRuntimeUuidPathInvalid, "parent path invalid for add entry");
		}
		if ( it->type == JsonPathComponent::PathObject ) {
			if ( !json_is_object(je) ) {
				throw Exception(ErrorRuntimeUuidPathInvalid, "parent path invalid for add entry");
			}
			parent_je = je;
		}
		else if (it->type == JsonPathComponent::PathArray ) {
			if (!json_is_array(je)) {
				throw Exception(ErrorRuntimeUuidPathInvalid, "parent path invalid for add entry");
			}
			int sz = json_array_size(je);
			if (sz == 0) {
				throw Exception(ErrorRuntimeUuidPathInvalid, "parent path invalid for add entry");
			}

			int idx = it->array_insert_idx;
			if (idx < 0) idx = sz + idx;
			if (idx < 0 || idx >= sz ) {
				throw Exception(ErrorRuntimeUuidPathInvalid, "parent path invalid for add entry");
			}

			parent_je = json_array_get(je, idx);
		}
	}

	if (!json_is_object(parent_je)) {
		throw Exception(ErrorRuntimeUuidPathInvalid, "parent path invalid for add entry");
	}

	const JsonPathComponent& lastPath = *(path.path.rbegin());
	if (lastPath.type == JsonPathComponent::PathArray) {
		json_t* cur = json_object_get(parent_je, lastPath.name.c_str());
		if ( !cur ) {
			cur = json_array();
			json_object_set_new(parent_je, lastPath.name.c_str(), cur);
		}

		if (!json_is_array(cur)) {
			throw Exception(ErrorRuntimeUuidPathInvalid, " path invalid for add entry");
		}
		else {
			int idx = lastPath.array_insert_idx;
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
				parse_struct(script_serialed, curPath, uuid_pathmap);
				curPath.push_back(lastPath.name.c_str(), idx);
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
			throw Exception(ErrorRuntimeUuidPathInvalid, " path invalid for add entry");
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
		const JsonPath& path, json_t* script_serialed, int give) throw(Exception)
{
	erase_runtime_entry(src_uuid);
	add_runtime_entry(path, script_serialed, give);
}

const JsonPath* MltRuntime::get_runtime_entry_path(const string& uuid)
{
	return NULL;
}

void MltRuntime::run() throw (Exception)
{

}

void MltRuntime::seek() throw (Exception)
{

}

void MltRuntime::stop() throw (Exception)
{

}

void MltRuntime::reload() throw (Exception)
{

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

bool MltRuntime::is_stopped()
{
}

bool MltRuntime::is_running()
{
}

NMSP_END(vedit)





