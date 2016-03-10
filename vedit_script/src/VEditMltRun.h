/*
 * VEditMltRun.h
 *
 *  Created on: 2016-2-26
 *      Author: li.lei@youku.com
 */

#ifndef VEDITMLTRUN_H_
#define VEDITMLTRUN_H_

#include "VEditCommon.h"
#include "VEditException.h"

NMSP_BEGIN(vedit)

class Script;

struct JsonPathComponent
{
	enum PathType {
		PathInvalid,
		PathObject,
		PathArray
	};

	JsonPathComponent(const char* desc = NULL);
	JsonPathComponent(const JsonPathComponent& r):
		type(r.type),
		name(r.name),
		array_insert_idx(r.array_insert_idx)
	{
	}

	JsonPathComponent& operator=(const JsonPathComponent& r) {
		type = r.type;
		name = r.name;
		array_insert_idx = r.array_insert_idx;
		return *this;
	}

	bool is_root()const {
		return type == PathObject && name.length() == 0;
	}

	PathType type;
	string name;
	int array_insert_idx;
};

struct JsonPath
{
	explicit JsonPath(const char* desc = NULL);
	JsonPath(const JsonPath& r);

	void push_back(const char* comp) {
		JsonPathComponent p;
		p.type = JsonPathComponent::PathObject;
		p.name = comp;
		path.push_back(p);
	}

	void push_back(const char* arrnm, int idx) {
		JsonPathComponent comp;
		comp.type = JsonPathComponent::PathArray;
		comp.name = arrnm;
		comp.array_insert_idx = idx;
		path.push_back(comp);
	}

	typedef list<JsonPathComponent>::iterator PathCompIter;
	typedef list<JsonPathComponent>::const_iterator PathCompCIter;
	list<JsonPathComponent> path;
	JsonPath& operator=(const JsonPath& r);

	static JsonPath operator+(const JsonPath& l, const JsonPath& r);
};

class MltRuntime
{
public:
	MltRuntime(json_t* script_serialed, int give=1) throw(Exception);
	virtual ~MltRuntime();

	/*
	 * 比如要修改一个playlist的第10个slice的第3个effects，如此表达
	 * "/slices:9/effects:3
	 */
	void erase_runtime_entry(const string& uuid) throw (Exception);
	void erase_runtime_entry(const JsonPath& path) throw(Exception);
	void erase_runtime_entry(const char* path) throw(Exception);

	void add_runtime_entry(
		const JsonPath& path,
		json_t* script_serialed, int give=1) throw(Exception);

	void replace_runtime_entry(const string& src_uuid,
		const JsonPath& path, json_t* script_serialed, int give = 1) throw(Exception);

	const JsonPath* get_runtime_entry_path(const string& uuid);

	void reload() throw (Exception);

	void run() throw (Exception);
	void seek() throw (Exception);
	void stop() throw (Exception);
	uint32_t get_frame_length() throw (Exception);
	uint32_t get_frame_position() throw (Exception);

	int get_runtime_entry_property_int(const string& uuid, const string& procname);
	int get_runtime_entry_property_double(const string& uuid, const string& procname);
	string get_runtime_entry_property_string(const string& uuid, const string& procname);

	static const char* const root_path = "";
	static const char* const attached_filter_path = "effects:-1";
	static const char* const playlist_entry_path = "slices:-1";
	static const char* const multitrack_entry_path = "tracks:-1";
private:
	friend class Script;

	void parse_struct(json_t* v, const JsonPath& curPath , hash_map<string,JsonPath>& uuid_paths) throw (Exception);

	uint32_t json_version;
	json_t* json_serialize;

	uint32_t producer_version;

	typedef hash_map<string,JsonPath>::iterator PathIter;
	typedef hash_map<string,JsonPath>::const_iterator PathCIter;
	typedef hash_map<string,mlt_service>::iterator SvcIter;
	typedef hash_map<string,mlt_service>::const_iterator SvcCIter;

	hash_map<string, JsonPath> uuid_pathmap;
	hash_map<string, mlt_service> uuid_mlt_svcmap;

	enum Status {
		StatusCreated,
		StatusInvalid,
		StatusInited,
		StatusRunning,
		StatusStopped
	};

	Status status;

	bool is_stopped();
	bool is_running();

	mlt_producer producer;
	mlt_consumer consumer;
};

typedef shared_ptr<MltRuntime> MltRuntimePtr;

NMSP_END(vedit)

#endif /* VEDITMLTRUN_H_ */
