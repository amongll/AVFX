/*
 * VEditCommon.h
 *
 *  Created on: 2016��2��22��
 *      Author: li.lei@youku.com
 */

#ifndef VEDITCOMMON_H_
#define VEDITCOMMON_H_
#include "VEditCommonDefine.h"
#include <string>
#include <tr1/memory>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <hash_map>
#include <hash_set>
#include <jansson.h>
#include <cassert>
#include <cstring>
#include <framework/mlt.h>
#include <pthread.h>
#include <iostream>

using namespace __gnu_cxx;
using namespace std::tr1;
using namespace std;

#define DECLARE_CONST_MEM_MODIFIER(mod,mem,type) \
	type mod = const_cast<type>(&(mem))

#define USE_CONST_MEM_MODIFIER(mod, mem, type) \
	mod = const_cast<type>(&(mem))




NMSP_BEGIN(vedit)

enum ScriptType {
	UNKNOWN_SCRIPT,
	AUDIO_RESOURCE_SCRIPT,
	VIDEO_RESOURCE_SCRIPT,
	IMAGE_RESOURCE_SCRIPT,
	GIF_RESOURCE_SCRIPT,
	FILTER_SCRIPT,
	PLAYLIST_SCRIPT,
	MULTITRACK_SCRIPT,
	TRANSITION_SCRIPT,
	INVALID_SCRIPT //占位符
};

bool is_valid_identifier(const char* check);

bool is_macro_only(const char* str);
bool is_selector_only(const char* str);

bool is_call(const char* str, string& procnm);

void parse_macros(const char* str, hash_multimap<string, int>& idxes, vector<string>& segments);
void parse_selector(const char* str, string& enm, string& snm);
void parse_params(const char* str, vector<string>& params);

void parse_params(const char* str, hash_multimap<string, int>& idxes, vector<string>& segments);
bool is_param_only(const char* str);


NMSP_END(vedit)

namespace __gnu_cxx {
template<>
struct hash<std::string>
{
	size_t operator()(const std::string& obj) const
	{
		return __stl_hash_string(obj.c_str());
	}
};

}

#endif /* VEDITCOMMON_H_ */
