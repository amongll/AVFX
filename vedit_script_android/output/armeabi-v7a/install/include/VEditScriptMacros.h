/*
 * VEditScriptMacros.h
 *
 *  Created on: 2016��2��23��
 *      Author: li.lei@youku.com
 *      Desc:	�궨�塣��ֻ֧�ֱ����������ַ�ֵ���Դ���ű�����Ҳ���Դ���ű�������ݵ�����
 */
#ifndef VEDITSCRIPTMACROS_INCLUDE
#define VEDITSCRIPTMACROS_INCLUDE

#include "VEditCommon.h"
#include "VEditDefinable.h"
#include "VEditCompilable.h"
#include "VEditUtil.h"

NMSP_BEGIN(vedit)

class Script;
class ScriptMacros :  public Definable
{
public:
	const json_t* get_macro(const char* nm) const {
		if (!nm || !strlen(nm) ) return NULL;
		MapCIter it = macros.find(nm);
		return it == macros.end() ? NULL : it->second.h;
	}
	virtual ~ScriptMacros();

private:

	/**
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
	};*/

	friend class Script;
	//hash_map<string,shared_ptr<JsonWrap> >	macros;
	//typedef hash_map<string, shared_ptr<JsonWrap> >::iterator MapIter;
	//typedef hash_map<string, shared_ptr<JsonWrap> >::const_iterator MapCIter;
	hash_map<string,JsonWrap>	macros;
	typedef hash_map<string, JsonWrap>::iterator MapIter;
	typedef hash_map<string, JsonWrap>::const_iterator MapCIter;

	ScriptMacros(Script& script) throw (Exception);
	void parse_macros() throw(Exception);

	json_t* define_copy;
	Script& parent;
};

typedef std::tr1::shared_ptr<ScriptMacros> ScriptMacrosPtr;

NMSP_END(vedit)

#endif

