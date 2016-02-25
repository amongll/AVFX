/*
 * VEditScriptMacros.h
 *
 *  Created on: 2016年2月23日
 *      Author: li.lei@youku.com
 *      Desc:	宏定义。宏只支持标量，其中字符串值可以带入脚本参数，也可以带入脚本输出内容的属性
 */

#include "VEditCommon.h"
#include "VEditDefinable.h"
#include "VEditCompilable.h"

NMSP_BEGIN(vedit)

class vedit::Script;
class ScriptMacros :  public Definable
{
public:
	const json_t* get_macro(const char* nm) const {
		if (!nm || strlen(nm) ) return NULL;
		MapCIter it = macros.find(nm);
		return it == macros.end() ? NULL : it->second->js;
	}
protected:
	friend class shared_ptr<ScriptMacros>;
	virtual ~ScriptMacros();

private:

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

	friend class vedit::Script;
	hash_map<string,shared_ptr<JsonWrap> >	macros;
	typedef hash_map<string, shared_ptr<JsonWrap> >::iterator MapIter;
	typedef hash_map<string, shared_ptr<JsonWrap> >::const_iterator MapCIter;

	ScriptMacros(const json_t& script) throw (Exception);
	void parse_macros() throw(Exception);

	json_t* define_copy;
	Script& parent;
};

typedef shared_ptr<ScriptMacros> ScriptMacrosPtr;

NMSP_END(vedit)


