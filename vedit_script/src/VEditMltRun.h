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
		PathObject,
		PathArray
	};

	JsonPathComponent(const char* desc = NULL);

	bool is_root()const {
		return type == PathObject && name.length() == 0;
	}

	PathType type;
	string name;
	int array_insert_idx;
};

struct JsonPath
{
	JsonPath(const char* desc = NULL);
	JsonPath(const JsonPath& r);
	vector<JsonPathComponent> path;
	JsonPath& operator=(const JsonPath& r);

	static JsonPath operator+(const JsonPath& l, const JsonPath& r);
};

class MltRuntime
{
public:
	MltRuntime(json_t* script_serialed, int give=1);
	virtual ~MltRuntime();

	/*
	 * 比如要修改一个playlist的第10个slice的第3个effects，如此表达
	 * "/slices:9/effects:3
	 */
	void erase_runtime_entry(const string& uuid);
	void erase_runtime_entry(const JsonPath& path);

	void add_runtime_entry(
		const JsonPath& path,
		json_t* script_serialed, int give=1);

	void replace_runtime_entry(const string& src_uuid,
		const JsonPath& path, json_t* script_serialed, int give = 1);

	const JsonPath* get_runtime_entry_path(const string& uuid);

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
	uint32_t json_version;
	json_t* json_serialize;

	uint32_t producer_version;

	hash_map<string, shared_ptr<JsonPath> > uuid_pathmap;
	hash_map<string, mlt_service> uuid_mlt_svcmap;

	mlt_producer producer;

	mlt_consumer consumer;
};

typedef shared_ptr<MltRuntime> MltRuntimePtr;

NMSP_END(vedit)

#endif /* VEDITMLTRUN_H_ */
