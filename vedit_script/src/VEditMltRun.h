/*
 * VEditMltRun.h
 *
 *  Created on: 2016-2-26
 *      Author: li.lei@youku.com
 */

#ifndef VEDITMLTRUN_H_
#define VEDITMLTRUN_H_

#include "VEditUtil.h"
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
		arr_idx(r.arr_idx)
	{
	}

	JsonPathComponent& operator=(const JsonPathComponent& r) {
		type = r.type;
		name = r.name;
		arr_idx = r.arr_idx;
		return *this;
	}

	bool is_root()const {
		return type == PathObject && name.length() == 0;
	}

	PathType type;
	string name;
	int arr_idx;
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
		comp.arr_idx = idx;
		path.push_back(comp);
	}

	string str()const;

	typedef list<JsonPathComponent>::iterator PathCompIter;
	typedef list<JsonPathComponent>::const_iterator PathCompCIter;
	list<JsonPathComponent> path;
	JsonPath& operator=(const JsonPath& r);
	JsonPath operator+(const JsonPath& r);

};

template<class SubClass>
struct MltLoaderDeclare
{
	static SubClass singleton;
	static int init_hack;

	static int _hack_init()
	{
		SubClass::declare();
		return 0;
	}
};

template<class SubClass>
SubClass MltLoaderDeclare<SubClass>::singleton;

struct MltLoader
{
	MltLoader(){}
	virtual ~MltLoader() {};

	typedef mlt_service (MltLoader::*LoadMltMemFp)(JsonWrap js);

	template<class SubClass>
	static void regist_loader(const char* procname,
			mlt_service (SubClass::*memfp)(JsonWrap)throw(Exception) )
	{
		static MltLoader* obj =
					dynamic_cast<MltLoader*>(&MltLoaderDeclare<SubClass>::singleton);
		assert(obj);
		pair<MltLoader*, LoadMltMemFp>& entry = loader_regists[string(procname)];
		entry.first = obj;
		entry.second = static_cast<LoadMltMemFp>(memfp);
	}

	static mlt_service load_mlt(JsonWrap js) throw (Exception);

	static hash_map<string, pair<MltLoader*,LoadMltMemFp> > loader_regists;
};

template<class SubClass>
int MltLoaderDeclare<SubClass>::init_hack = MltLoaderDeclare<SubClass>::_hack_init();

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

	void add_runtime_entry(
		const JsonPath& path,
		json_t* script_serialed, int give=1) throw(Exception);

	void replace_runtime_entry(const JsonPath& path, json_t* script_seriled, int giv=1)
		throw (Exception);
	void replace_runtime_entry(const string& src_uuid,
		json_t* script_serialed, int give = 1) throw(Exception);

	const JsonPath* get_runtime_entry_path(const string& uuid);

	json_t* run() throw (Exception);
	void seek(int framePos) throw (Exception);
	void stop() throw (Exception);

	bool running() {
		Lock lk(&run_lock);
		return status == StatusRunning;
	}

	uint32_t get_frame_length() throw (Exception);
	uint32_t get_frame_position() throw (Exception);

	int get_runtime_entry_property_int(const string& uuid, const string& procname);
	int get_runtime_entry_property_double(const string& uuid, const string& procname);
	string get_runtime_entry_property_string(const string& uuid, const string& procname);

private:
	void stop_ulk() throw (Exception);

	friend class Script;

	void parse_struct(json_t* v, const JsonPath& curPath ,
		hash_map<string,JsonPath>& uuid_paths, int erase = 0) throw (Exception);

	uint32_t json_version;
	json_t* json_serialize;

	uint32_t producer_version;

	typedef hash_map<string,JsonPath>::iterator PathIter;
	typedef hash_map<string,JsonPath>::const_iterator PathCIter;
	typedef hash_map<string,mlt_service>::iterator SvcIter;
	typedef hash_map<string,mlt_service>::const_iterator SvcCIter;

	pthread_mutex_t run_lock;

	hash_map<string, JsonPath> uuid_pathmap;
	hash_map<string, mlt_service> uuid_mlt_svcmap;

	enum Status {
		StatusCreated,
		StatusLoadFailed,
		StatusRunning,
		StatusStopped
	};

	Status status;


	mlt_producer producer;
	mlt_consumer consumer;
};

typedef shared_ptr<MltRuntime> MltRuntimePtr;

NMSP_END(vedit)

#endif /* VEDITMLTRUN_H_ */
