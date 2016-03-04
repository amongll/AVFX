/*
 * VEditScriptEnums.h
 *
 *  Created on: 2016��2��22��
 *      Author: li.lei@youku.com
 */

#ifndef VEDITSCRIPTENUMS_H_
#define VEDITSCRIPTENUMS_H_

#include "VEditCommon.h"
#include "VEditDefinable.h"
#include "VEditCompilable.h"

NMSP_BEGIN(vedit)

class Script;
class ScriptEnums : public Definable
{
public:
	const json_t* get_selector(const char* enum_name, const char* sel_name) const {
		if (!enum_name || !strlen(enum_name) || !sel_name || !strlen(sel_name))
			return NULL;
		MapCIter it = enums.find(string(enum_name));
		if (it == enums.end() ) return NULL;
		EnumCIter it2 = it->second.find(string(sel_name));
		return it2 == it->second.end() ? NULL : it2->second->js;
	}

	bool has_enum(const char* name) const {
		if (!name || !strlen(name)) return false;
		MapCIter it = enums.find(name);
		if ( it == enums.end() ) return false;
		return true;
	}

	virtual ~ScriptEnums() ;
private:
	friend class Script;
	ScriptEnums(Script& script) throw (Exception);

	void parse_enums() throw (Exception);

	struct JsonWrap {
		JsonWrap( json_t* detail) throw (Exception);
		~JsonWrap() {
			json_decref(js);
		}
		JsonWrap(const JsonWrap& rh):js(NULL) {
			js = json_incref((json_t*)rh.js);
		}
		JsonWrap& operator=(const JsonWrap& rh) {
			if ( js ) json_decref(js);
			js = json_incref((json_t*)rh.js);
			return *this;
		}
		json_t* js;
	};

	json_t* defines;
	Script& parent;
	typedef hash_map<string, shared_ptr<JsonWrap> > Enum;
	typedef Enum::iterator EnumIter;
	typedef Enum::const_iterator EnumCIter;
	hash_map<string, Enum>  enums;
	typedef hash_map<string, Enum >::iterator MapIter;
	typedef hash_map<string, Enum >::const_iterator MapCIter;
};

typedef shared_ptr<ScriptEnums> ScriptEnumsPtr;

NMSP_END(vedit)



#endif /* VEDITSCRIPTENUMS_H_ */
